#ifndef SERIALPORT_H
#define SERIALPORT_H

// 端口管理
#include <QtSerialPort>
#include <QVector>
#include <String>

class serialport
{
public:
    serialport();
    ~serialport();

    QSerialPort*  InitSerialPort(QString,int); // 初始化串口
    QVector<QString> ScanSerialPort();         // 扫描电脑串口

    bool GetPortStatu(QString);                // 获取串口状态
    bool PortIsOpen();                         // 判断串口是否打开
    QByteArray ReadMsgForPort();               // 从串口读取信息
    void SendMsgToPort(QString);               // 发送字符串
    void Send_TimeMs(int);                     // 发送时间
    void Send_Data(int);                       // 发送数据
    void Send_Com(int,int);                    // 发送命令
    void Send_Angle(int,int);                  // 发送倾角传感器角度
    void ClosePort();                          // 关闭串口

private:
    QSerialPort *port = NULL;
    QByteArray requestData;
    QVector<QString> portList;
    bool potStatu = true;
    char buf[80];
};

#endif // SERIALPORT_H
