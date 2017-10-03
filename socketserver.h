#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include "serialport.h"
#include <QObject>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class socketserver : public QObject
{
    Q_OBJECT
public:
    explicit socketserver(serialport *serial,quint16 port, bool debug = false, QObject *parent = Q_NULLPTR);
    ~socketserver();
    // 发送Socket信息
    void SendSocketMessage(int,int,int,int);

Q_SIGNALS:
    void closed();

private Q_SLOTS:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();

private:
    serialport *serialPort;
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
    QWebSocket *globalUser = NULL;
    bool m_debug;

    // json 字符串转换
    QString QjsonObjectToQString(const QJsonObject& jsonObject);
    QJsonObject QStringToQjsonObject(const QString& jsonString);

    void FromAppCommand(QJsonObject);
};

#endif // SOCKETSERVER_H
