#include "DataSimulator.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QRandomGenerator>
#include <QDebug>

DataSimulator::DataSimulator(QObject *parent)
    : QObject(parent), m_isRunning(false), m_connectionStatus("未连接"), m_tcpSocket(new QTcpSocket(this)), m_currentOrderId(""), m_targetTemp(4.0)
{
    // 初始化网络相关
    connect(&m_timer, &QTimer::timeout, this, &DataSimulator::generateData);
    connect(m_tcpSocket, &QTcpSocket::connected, this, &DataSimulator::onConnected);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &DataSimulator::onDisconnected);
    connect(m_tcpSocket, &QTcpSocket::errorOccurred, this, &DataSimulator::onErrorOccurred);

    // 初始化大模型服务
    m_llmService = new LLMService(this);
    connect(m_llmService, &LLMService::dataReceived, this, &DataSimulator::onLLMDataReceived);
    connect(m_llmService, &LLMService::errorOccurred, this, &DataSimulator::onLLMError);

    // 初始化真实 GPS 模块
    m_geoSource = QGeoPositionInfoSource::createDefaultSource(this);
    if (m_geoSource) {
        // 设置定位精度和更新间隔 (这里设为 1 分钟更新一次)
        m_geoSource->setUpdateInterval(60000); 
        connect(m_geoSource, &QGeoPositionInfoSource::positionUpdated, this, &DataSimulator::onPositionUpdated);
        qDebug() << "GPS Module initialized successfully.";
    } else {
        qWarning() << "Failed to initialize GPS Module! Will fallback to simulated GPS.";
    }
}

DataSimulator::~DataSimulator()
{
    if (m_tcpSocket->isOpen()) {
        m_tcpSocket->disconnectFromHost();
    }
}

bool DataSimulator::isRunning() const
{
    return m_isRunning;
}

QString DataSimulator::currentJson() const
{
    return m_currentJson;
}

QString DataSimulator::connectionStatus() const
{
    return m_connectionStatus;
}

void DataSimulator::startSimulation(const QString &orderId, double targetTemp)
{
    if (!m_isRunning) {
        m_isRunning = true;
        m_currentOrderId = orderId;
        m_targetTemp = targetTemp;
        
        emit isRunningChanged();
        
        m_connectionStatus = "正在连接后端...";
        emit connectionStatusChanged();
        
        // 尝试连接到 Reactor 后端 (假设本机运行，端口 8888)
        // 实际部署时可修改为服务器真实 IP
        m_tcpSocket->connectToHost("127.0.0.1", 8888);
        
        // 开启真实 GPS 定位 (如果有硬件)
        if (m_geoSource) {
            m_geoSource->startUpdates();
            qDebug() << "Started real GPS tracking.";
        }

        // 立即生成一次数据，然后每 1 分钟 (60000ms) 触发一次大模型生成
        generateData();
        m_timer.start(60000); 
    }
}

void DataSimulator::stopSimulation()
{
    if (m_isRunning) {
        m_isRunning = false;
        m_timer.stop();
        
        if (m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
            m_tcpSocket->disconnectFromHost();
        }
        
        // 停止真实 GPS
        if (m_geoSource) {
            m_geoSource->stopUpdates();
            qDebug() << "Stopped real GPS tracking.";
        }

        m_connectionStatus = "已停止";
        emit connectionStatusChanged();
        emit isRunningChanged();
    }
}

void DataSimulator::onConnected()
{
    m_connectionStatus = "已连接到服务器";
    emit connectionStatusChanged();
    qDebug() << "Connected to server!";
}

void DataSimulator::onDisconnected()
{
    m_connectionStatus = "已断开连接";
    emit connectionStatusChanged();
    qDebug() << "Disconnected from server!";
}

void DataSimulator::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    m_connectionStatus = "连接错误: " + m_tcpSocket->errorString();
    emit connectionStatusChanged();
    qDebug() << "Socket error:" << m_tcpSocket->errorString();
}

void DataSimulator::onPositionUpdated(const QGeoPositionInfo &info)
{
    if (info.isValid()) {
        m_longitude = info.coordinate().longitude();
        m_latitude = info.coordinate().latitude();
        emit locationUpdated(m_latitude, m_longitude);
        qDebug() << "Real GPS location updated:" << m_longitude << "," << m_latitude;
    }
}

void DataSimulator::generateData()
{
    // 1. 获取当前经纬度 (如果有真实 GPS 模块并成功定位，则 m_longitude 已经被 onPositionUpdated 更新)
    // 如果没有真实硬件 (如在电脑上测试)，我们依然保留一点点微小的随机位移，以便于在地图上看出“正在移动”
    if (!m_geoSource || !m_geoSource->lastKnownPosition().isValid()) {
        m_longitude += QRandomGenerator::global()->generateDouble() * 0.01;
        m_latitude -= QRandomGenerator::global()->generateDouble() * 0.01;
        qDebug() << "Using simulated GPS for movement.";
    }

    // 2. 调用大模型 API 获取环境数据
    // 注意：大模型请求是异步的，真正的组装 JSON 和发送逻辑移到了 onLLMDataReceived 中
    qDebug() << "Requesting environment data from GLM API...";
    m_llmService->fetchEnvironmentData(m_longitude, m_latitude, m_targetTemp);
}

void DataSimulator::onLLMDataReceived(const QString &weather, const QString &humidity, double currentTemp, bool isAlert)
{
    // 3. 大模型返回数据后，组装完整的 JSON
    QJsonObject jsonObj;
    jsonObj["order_id"] = m_currentOrderId;
    jsonObj["timestamp"] = QDateTime::currentSecsSinceEpoch();
    
    QJsonObject locationObj;
    locationObj["longitude"] = m_longitude;
    locationObj["latitude"] = m_latitude;
    locationObj["city"] = "AI_Generated"; // 大模型不一定返回城市名，这里可以暂时省略或后续让大模型一并返回
    locationObj["weather"] = weather;
    locationObj["outside_humidity"] = humidity;
    jsonObj["location"] = locationObj;

    QJsonObject cargoObj;
    cargoObj["target_temp"] = m_targetTemp;
    cargoObj["current_temp"] = currentTemp;
    cargoObj["is_alert"] = isAlert;
    jsonObj["cargo_status"] = cargoObj;

    QJsonDocument doc(jsonObj);
    m_currentJson = QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
    emit currentJsonChanged();
    emit dataGenerated(m_currentJson);

    // 如果已连接到服务器，则发送数据
    if (m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        QByteArray payload = doc.toJson(QJsonDocument::Compact);
        payload.append('\n'); 
        m_tcpSocket->write(payload);
        m_tcpSocket->flush();
        qDebug() << "Sent data to server:" << payload;
    }
}

void DataSimulator::onLLMError(const QString &errorMsg)
{
    qWarning() << "GLM API Error:" << errorMsg;
    // TODO: 如果大模型请求失败，可以在这里写一个 fallback 逻辑，比如使用之前的纯本地随机模拟数据
}
