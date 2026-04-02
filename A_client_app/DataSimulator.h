#ifndef DATASIMULATOR_H
#define DATASIMULATOR_H

#include <QObject>
#include <QTimer>
#include <QJsonObject>
#include <QTcpSocket>
#include <QGeoPositionInfoSource>
#include <QGeoPositionInfo>

#include "LLMService.h"
#include "OrderDatabase.h"

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
    void locationUpdated(double lat, double lon); // 新增信号通知UI位置更新

private slots:
    void generateData();
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void onPositionUpdated(const QGeoPositionInfo &info); // 接收真实GPS回调
    
    // 接收大模型返回的数据
    void onLLMDataReceived(const QString &weather, const QString &humidity, double currentTemp, bool isAlert);
    void onLLMError(const QString &errorMsg);

private:
    QTimer m_timer;
    bool m_isRunning;
    QString m_currentJson;
    QString m_connectionStatus;
    
    QTcpSocket *m_tcpSocket;
    QGeoPositionInfoSource *m_geoSource; // 真实GPS源
    LLMService *m_llmService;            // 大模型服务

    // 当前绑定的订单信息
    QString m_currentOrderId;
    double m_targetTemp;

    // 当前经纬度
    double m_longitude = 116.407396; // 默认北京，获取到真实GPS后更新
    double m_latitude = 39.904200;
    
    OrderDatabase m_dbHelper;
};

#endif // DATASIMULATOR_H
