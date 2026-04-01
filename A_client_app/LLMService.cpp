#include "LLMService.h"
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

LLMService::LLMService(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &LLMService::onReplyFinished);
}

void LLMService::fetchEnvironmentData(double longitude, double latitude, double targetTemp)
{
    // 智谱 API (GLM-4) 的端点
    QUrl url("https://open.bigmodel.cn/api/paas/v4/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());

    // 构建 Prompt
    QString prompt = QString(
        "你现在是一个冷链车环境数据生成器。当前车辆所在的经纬度是 [%1, %2]。"
        "请根据这个地理位置（如果在中国范围内，请推测合理的天气），并结合车辆的目标温度 [%3℃]，"
        "为我生成一段 JSON 格式的模拟数据。不要返回任何其他解释文字，只返回纯 JSON。"
        "必须包含以下字段："
        "1. 'weather' (字符串，如'晴', '大雨')"
        "2. 'humidity' (字符串，如'65%')"
        "3. 'current_temp' (浮点数，要求在目标温度 [%3℃] 的上下 1℃ 波动，但你有 5% 的概率模拟出冷机故障，温度飙升到 15℃ 左右)"
        "4. 'is_alert' (布尔值，如果 current_temp 超过目标温度 5℃ 以上，设为 true，否则为 false)"
    ).arg(longitude).arg(latitude).arg(targetTemp);

    // 组装智谱 API 的请求体
    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = prompt;

    QJsonArray messagesArray;
    messagesArray.append(messageObj);

    QJsonObject requestObj;
    requestObj["model"] = "glm-4";
    requestObj["messages"] = messagesArray;
    requestObj["temperature"] = 0.7; // 增加一点随机性

    QJsonDocument doc(requestObj);
    QByteArray postData = doc.toJson();

    m_networkManager->post(request, postData);
    qDebug() << "Sent request to GLM API for location:" << longitude << "," << latitude;
}

void LLMService::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject rootObj = doc.object();

        // 提取智谱 API 的回复内容
        if (rootObj.contains("choices") && rootObj["choices"].isArray()) {
            QJsonArray choices = rootObj["choices"].toArray();
            if (!choices.isEmpty()) {
                QJsonObject message = choices[0].toObject()["message"].toObject();
                QString content = message["content"].toString();
                
                // 清理大模型可能返回的 markdown 标记 (如 ```json ... ```)
                content.remove("```json\n");
                content.remove("```");
                content = content.trimmed();

                // 解析生成的 JSON 数据
                QJsonParseError jsonError;
                QJsonDocument generatedDoc = QJsonDocument::fromJson(content.toUtf8(), &jsonError);
                
                if (jsonError.error == QJsonParseError::NoError && generatedDoc.isObject()) {
                    QJsonObject genObj = generatedDoc.object();
                    
                    QString weather = genObj["weather"].toString("未知");
                    QString humidity = genObj["humidity"].toString("50%");
                    double currentTemp = genObj["current_temp"].toDouble();
                    bool isAlert = genObj["is_alert"].toBool();
                    
                    qDebug() << "Successfully parsed GLM generated data:" << weather << currentTemp << "Alert:" << isAlert;
                    
                    emit dataReceived(weather, humidity, currentTemp, isAlert);
                } else {
                    emit errorOccurred("Failed to parse GLM JSON output: " + jsonError.errorString());
                    qDebug() << "GLM Content was:" << content;
                }
            }
        }
    } else {
        emit errorOccurred("Network Error: " + reply->errorString());
        qDebug() << "GLM API Request failed:" << reply->errorString() << reply->readAll();
    }
    reply->deleteLater();
}
