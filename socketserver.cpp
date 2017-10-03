#include "socketserver.h"
#include "serialport.h"
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include <QtCore/QDebug>
#include <QJsonDocument>
#include <QDebug>

QT_USE_NAMESPACE

socketserver::socketserver(serialport *serial,quint16 port, bool debug, QObject *parent) :
    QObject(parent),
    m_pWebSocketServer(new QWebSocketServer(QStringLiteral("Echo Server"),
                                            QWebSocketServer::NonSecureMode, this)),
    m_debug(debug)
{
    // 启动Socket服务时，传入串口实例
    serialPort = serial;

    if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
        if (m_debug)
            qDebug() << "socketserver listening on port" << port;
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
                this, &socketserver::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &socketserver::closed);
    }
}

socketserver::~socketserver()
{
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

// 创建一个新的连接
void socketserver::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &socketserver::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &socketserver::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &socketserver::socketDisconnected);
    m_clients << pSocket;
    globalUser = pSocket;
}

// 接收来自 Web 的信息
void socketserver::processTextMessage(QString message)
{
    QJsonObject AppData = this->QStringToQjsonObject(message);
    this->FromAppCommand(AppData);
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "客户端推送:" << message;
    if (pClient) {
        pClient->sendBinaryMessage("客户端推送:");
    }
}

// 接收二进制消息
void socketserver::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "Binary Message received:" << message;
    if (pClient) {
        pClient->sendBinaryMessage(message);
    }
}

// 断开Socket连接
void socketserver::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "socketDisconnected:" << pClient;
    if (pClient) {
        qDebug()<< "断开socket连接";
        m_clients.removeAll(pClient);
        globalUser = NULL;
        pClient->deleteLater();
    }
}


// 服务器推送信息
void socketserver::SendSocketMessage(int msgType,int comType,int com,int data)
{
    // 发送单片机状态到App
    QJsonObject sendjson;
    sendjson.insert("msgType",msgType);
    sendjson.insert("comType",comType);
    sendjson.insert("com", com);
    sendjson.insert("data", data);

    if(globalUser == NULL) return;

    QWebSocket *pClient = qobject_cast<QWebSocket *>(globalUser);
    if (pClient) {
        qDebug()<<this->QjsonObjectToQString(sendjson);
        pClient->sendTextMessage(this->QjsonObjectToQString(sendjson));
    }
}

// 处理App发过来的数据
void socketserver::FromAppCommand(QJsonObject json)
{
    int msgType = json.value("msgType").toInt();
    int comType = json.value("comType").toInt();
    int com = json.value("com").toInt();
    int data = json.value("data").toInt();

    // 发送命令到单片机
    switch (msgType) {
    case 0x01:
         serialPort->Send_Com(comType,com);
        break;
    case 0x02:
         serialPort->Send_Data(data);
        break;
    default:
        break;
    }

    // 发送单片机状态到App
    this->SendSocketMessage(msgType,comType,com,data);
}

// JsonObject 转 QString
QString socketserver::QjsonObjectToQString(const QJsonObject &jsonObject)
{
    return QString(QJsonDocument(jsonObject).toJson());
}

// QString 转 QJsonObject
QJsonObject socketserver::QStringToQjsonObject(const QString &jsonString)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonString.toLocal8Bit().data());
    if( jsonDocument.isNull() ){
        qDebug()<< "输入字符串格式不正确 "<< jsonString.toLocal8Bit().data();
    }
    QJsonObject jsonObject = jsonDocument.object();
    return jsonObject;
}

