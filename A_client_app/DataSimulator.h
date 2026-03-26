#ifndef DATASIMULATOR_H
#define DATASIMULATOR_H

#include <QObject>
#include <QTimer>
#include <QJsonObject>
#include <QTcpSocket>

class DataSimulator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
    Q_PROPERTY(QString currentJson READ currentJson NOTIFY currentJsonChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)

public:
    explicit DataSimulator(QObject *parent = nullptr);
    ~DataSimulator();

    bool isRunning() const;
    QString currentJson() const;
    QString connectionStatus() const;

    Q_INVOKABLE void startSimulation(const QString &orderId, double targetTemp);
    Q_INVOKABLE void stopSimulation();

signals:
    void isRunningChanged();
    void currentJsonChanged();
    void connectionStatusChanged();
    void dataGenerated(const QString &jsonString);

private slots:
    void generateData();
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    QTimer m_timer;
    bool m_isRunning;
    QString m_currentJson;
    QString m_connectionStatus;
    
    QTcpSocket *m_tcpSocket;

    // 当前绑定的订单信息
    QString m_currentOrderId;
    double m_targetTemp;

    // 模拟的起始经纬度 (北京)
    double m_longitude = 116.407396;
    double m_latitude = 39.904200;
};

#endif // DATASIMULATOR_H
