#ifndef WIDGET_H
#define WIDGET_H

#include "serialport.h"
#include "socketserver.h"
#include <QWidget>
#include <QVector>
#include <QtSerialPort> // Qt串口调试类
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QtWebSockets/QtWebSockets>

namespace Ui {
    class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    void closeEvent(QCloseEvent *event); // 拦截窗口关闭信号

private:
    void InitRightButtomIcon();         // 初始化托盘图标

private:
    Ui::Widget *ui;
    socketserver *server = NULL;        // socket服务
    QString requestDataStr;             // 以字符串形式接收
    QSerialPort *my_serialPort = NULL;  // 串口指针
    QByteArray requestData;             // 串口返回数据容器
    int ResponseData;
    QTimer *my_timer;
    QVector<QString> portList;          // 扫描串口列表
    QSystemTrayIcon *m_trayIcon;        // 系统右下角图标
    QMenu *menu;                        // 托盘图标菜单
    QAction *exitAction;
    QAction *helpAction;

    serialport *myPort;                 // 串口操作句柄


private slots:
    void readMyCom();
    void on_rotaryPulse_valueChanged(int arg1);

    void on_refreshButton_clicked();

    void on_pushButton_comopen_clicked();

    void on_pushButton_send_clicked();
    // 响应右下角图标点击
    void slotIconActivated(QSystemTrayIcon::ActivationReason);
    void exitSerialPort();
    void helpMe();
    void on_ctrMotor_clicked();
    void on_rotaryLeft_clicked();
    void on_rotaryRight_clicked();
    void on_clearMsg_clicked();
    void on_rotaryAnlge_valueChanged(int arg1);
    void on_rotaryTime_valueChanged(int arg1);
    void on_fanController_clicked();
    void on_fanEast_clicked();
    void on_fanSouth_clicked();
    void on_fanWest_clicked();
    void on_fanNorth_clicked();
    void on_spinBox_X_valueChanged(int arg1);
    void on_spinBox_Y_valueChanged(int arg1);
    void on_spinBox_Z_valueChanged(int arg1);
    void on_fanWestNorth_clicked(); /* 电机转动方向控制 */
    void on_fanEastNorth_clicked();
    void on_fanEastSouth_clicked();
    void on_fanWestSouth_clicked();
    void on_startSocket_clicked();  /* Socket服务 */
    void on_refresIpv4_clicked();
    void on_refreshButton_clicked(bool checked);
};

#endif // WIDGET_H
