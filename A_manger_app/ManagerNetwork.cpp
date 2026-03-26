#include "ManagerNetwork.h"
#include <QDebug>

ManagerNetwork::ManagerNetwork(QObject *parent)
    : QObject(parent), m_tcpSocket(new QTcpSocket(this)), m_connectionStatus("未连接")
{
    connect(m_tcpSocket, &QTcpSocket::connected, this, &ManagerNetwork::onConnected);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &ManagerNetwork::onDisconnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &ManagerNetwork::onReadyRead);
    connect(m_tcpSocket, &QTcpSocket::errorOccurred, this, &ManagerNetwork::onErrorOccurred);
}

ManagerNetwork::~ManagerNetwork()
{
    if (m_tcpSocket->isOpen()) {
        m_tcpSocket->disconnectFromHost();
    }
}

QString ManagerNetwork::connectionStatus() const
{
    return m_connectionStatus;
}

void ManagerNetwork::connectToServer()
{
    if (m_tcpSocket->state() != QAbstractSocket::ConnectedState) {
        m_connectionStatus = "正在连接...";
        emit connectionStatusChanged();
        
        // 假设 Reactor 后端在本地 8888 端口运行
        m_tcpSocket->connectToHost("127.0.0.1", 8888);
    }
}

void ManagerNetwork::disconnectFromServer()
{
    if (m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        m_tcpSocket->disconnectFromHost();
    }
}

void ManagerNetwork::onConnected()
{
    m_connectionStatus = "已连接到服务器";
    emit connectionStatusChanged();
    qDebug() << "Manager connected to server.";
}

void ManagerNetwork::onDisconnected()
{
    m_connectionStatus = "未连接";
    emit connectionStatusChanged();
    qDebug() << "Manager disconnected from server.";
}

void ManagerNetwork::onReadyRead()
{
    // 读取所有可用的数据
    // 假设后端使用 '\n' 作为消息分隔符
    while (m_tcpSocket->canReadLine()) {
        QByteArray line = m_tcpSocket->readLine().trimmed();
        if (!line.isEmpty()) {
            QString jsonString = QString::fromUtf8(line);
            emit truckDataReceived(jsonString);
        }
    }
}

void ManagerNetwork::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    m_connectionStatus = "连接错误: " + m_tcpSocket->errorString();
    emit connectionStatusChanged();
    qDebug() << "Manager socket error:" << m_tcpSocket->errorString();
}
