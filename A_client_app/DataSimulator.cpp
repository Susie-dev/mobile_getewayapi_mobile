#include "DataSimulator.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QRandomGenerator>
#include <QDebug>

DataSimulator::DataSimulator(QObject *parent)
    : QObject(parent), m_isRunning(false), m_connectionStatus("未连接"), m_tcpSocket(new QTcpSocket(this)), m_currentOrderId(""), m_targetTemp(4.0)
{
    connect(&m_timer, &QTimer::timeout, this, &DataSimulator::generateData);
    
    // 连接 QTcpSocket 的信号
    connect(m_tcpSocket, &QTcpSocket::connected, this, &DataSimulator::onConnected);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &DataSimulator::onDisconnected);
    connect(m_tcpSocket, &QTcpSocket::errorOccurred, this, &DataSimulator::onErrorOccurred);
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
        
        // 立即生成一次，然后每 3 秒生成一次
        generateData();
        m_timer.start(3000); 
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

void DataSimulator::generateData()
{
    // 1. 模拟位置移动 (每次向南/向东微小移动)
    m_longitude += QRandomGenerator::global()->generateDouble() * 0.01;
    m_latitude -= QRandomGenerator::global()->generateDouble() * 0.01;

    // 2. 模拟车内温度 (目标温度，上下波动)
    double temp = m_targetTemp + (QRandomGenerator::global()->generateDouble() * 2.0 - 1.0);
    
    // 5%的概率出现冷机故障，温度飙升
    bool isAlert = false;
    if (QRandomGenerator::global()->bounded(100) < 5) {
        temp = m_targetTemp + 8.0 + QRandomGenerator::global()->generateDouble() * 5.0; // 远超目标温度
        isAlert = true;
    }

    // 3. 组装 JSON
    QJsonObject jsonObj;
    jsonObj["order_id"] = m_currentOrderId;
    jsonObj["timestamp"] = QDateTime::currentSecsSinceEpoch();
    
    QJsonObject locationObj;
    locationObj["longitude"] = m_longitude;
    locationObj["latitude"] = m_latitude;
    locationObj["city"] = "运输途中";
    locationObj["weather"] = "晴";
    locationObj["outside_humidity"] = "45%";
    jsonObj["location"] = locationObj;

    QJsonObject cargoObj;
    cargoObj["target_temp"] = m_targetTemp;
    cargoObj["current_temp"] = QString::number(temp, 'f', 1).toDouble();
    cargoObj["is_alert"] = isAlert;
    jsonObj["cargo_status"] = cargoObj;

    QJsonDocument doc(jsonObj);
    m_currentJson = QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
    emit currentJsonChanged();
    emit dataGenerated(m_currentJson);

    // 如果已连接到服务器，则发送数据
    if (m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        // 将 JSON 数据转为紧凑格式发送，节省带宽
        QByteArray payload = doc.toJson(QJsonDocument::Compact);
        payload.append('\n'); // 添加换行符作为包分隔符，方便后端解析
        m_tcpSocket->write(payload);
        m_tcpSocket->flush();
        qDebug() << "Sent data to server:" << payload;
    }
}
