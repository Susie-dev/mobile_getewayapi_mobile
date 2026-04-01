#ifndef LLMSERVICE_H
#define LLMSERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class LLMService : public QObject
{
    Q_OBJECT
public:
    explicit LLMService(QObject *parent = nullptr);

    // 调用智谱 API 获取环境数据
    void fetchEnvironmentData(double longitude, double latitude, double targetTemp);

signals:
    // 当获取成功时触发，返回解析好的 JSON 字符串
    void dataReceived(const QString &weather, const QString &humidity, double currentTemp, bool isAlert);
    void errorOccurred(const QString &errorMsg);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_networkManager;
    const QString m_apiKey = "0cbb811830384d32baa6f762a1919871.EZ1Xih3Gl2TH2HUU"; // 您提供的 API Key
};

#endif // LLMSERVICE_H
