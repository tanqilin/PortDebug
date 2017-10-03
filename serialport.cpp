#include "serialport.h"
#include <QtSerialPort>
#include <String>
#include <QDebug>
#include <algorithm>
#include <math.h>

serialport::serialport()
{

}

serialport::~serialport()
{
    delete port;
}

// 扫描串口
QVector<QString> serialport::ScanSerialPort()
{

    if(portList.length() > 0) portList.clear();

    foreach( const QSerialPortInfo &Info,QSerialPortInfo::availablePorts())
    {
        qDebug() << "portName    :"  << Info.portName();//串口号
        qDebug() << "Description   :" << Info.description(); // 串口描述
        qDebug() << "Manufacturer:" << Info.manufacturer();

        QSerialPort serial;
        serial.setPort(Info);

        if( serial.open( QIODevice::ReadWrite) )//如果串口是可以读写方式打开的
        {
            portList.append(Info.portName()); // 向数组添加能用的串口
            serial.close();
        }
    }

    return portList;
}

// 打开串口
QSerialPort* serialport::InitSerialPort(QString portName,int baudRate)
{
    //对串口进行一些初始化
    port = new QSerialPort();
    port->setPortName( portName );  // 串口名
    port->open( QIODevice::ReadWrite );
    port->setBaudRate( baudRate );//波特率
    port->setDataBits( QSerialPort::Data8 );//数据字节，8字节
    port->setParity( QSerialPort::NoParity );//校验，无
    port->setFlowControl( QSerialPort::NoFlowControl );//数据流控制,无
    port->setStopBits( QSerialPort::OneStop );//一位停止位

    return port;
}

// 串口状态
bool serialport::GetPortStatu(QString portName)
{
    port = new QSerialPort();
    port->setPortName( portName );
    potStatu = port->open( QIODevice::ReadWrite );
    port->close();

    // true 串口可以使用，false 串口被占用
    return potStatu;
}

// 串口运行状态
bool serialport::PortIsOpen()
{
    // 如果对象port还没实例化
    if(port == NULL) return false;

    return port->isOpen();
}

// 读取串口数据
QByteArray serialport::ReadMsgForPort()
{
    requestData = port->readAll();
    return requestData;
}

// 发送字符串
void serialport::SendMsgToPort(QString hex)
{
    if(!port->isOpen()) return;

    port->write(hex.toLatin1());
}


// 发送脉冲事件 毫秒
void serialport::Send_TimeMs(int ms)
{
    if(!port->isOpen()) return;

    int hex_h = (ms&0xff00)>>8;   // 高八位
    int hex_l = (ms&0x00ff);        // 低八位

    if(ms>255){
        const unsigned char s[4]={0x03,hex_h,hex_l,0xFF};// 大于255时
        port->write((char *)s);
    }
    else{
        const unsigned char s[4]={0x03,0x03,ms,0xFF};
        port->write((char *)s);
    }
}

// 发送脉冲数据
void serialport::Send_Data(int data)
{
    if(!port->isOpen()) return;
    QString hex = "0x02"+QString("%1").arg(data,4,16,QLatin1Char('0'));

    int hex_h = (data&0xff00)>>8;   // 高八位
    int hex_l = (data&0x00ff);        // 低八位

    if(data>255){
        const unsigned char s[4]={0x02,hex_h,hex_l,0xFF};// 大于255时
        port->write((char *)s);
    }
    else{
        const unsigned char s[4]={0x02,0x02,data,0xFF};
        port->write((char *)s);
    }
    itoa(data,buf,16);
}

// 发送命令
void serialport::Send_Com(int type,int com)
{
    if(!port->isOpen()) return;

    QString hex = "0x01"+QString("%1").arg(com,4,16,QLatin1Char('0'));

    const unsigned char s[4]={0x01,type,com,0xFF};// 0x014F

    port->write((char*)s);
}

// 发送倾角传感器控制角度
void serialport::Send_Angle(int type, int angle)
{
    if(!port->isOpen()) return;

    const unsigned char s[4]={0x04,type,angle,0xFF};// 0x014F

    port->write((char*)s);
}

// 关闭串口
void serialport::ClosePort()
{
    port->close();
}


