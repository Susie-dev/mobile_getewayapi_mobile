#ifndef MANAGERNETWORK_H
#define MANAGERNETWORK_H

#include <QObject>
#include <QTcpSocket>

class ManagerNetwork : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)

public:
    explicit ManagerNetwork(QObject *parent = nullptr);
    ~ManagerNetwork();

    QString connectionStatus() const;

    Q_INVOKABLE void connectToServer();
    Q_INVOKABLE void disconnectFromServer();

signals:
    void connectionStatusChanged();
    void truckDataReceived(const QString &jsonString);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *m_tcpSocket;
    QString m_connectionStatus;
};

#endif // MANAGERNETWORK_H
