#include "widget.h"
#include "ui_widget.h"
#include "serialport.h"
#include "socketserver.h"
#include "stdio.h"

#include <QSystemTrayIcon>
#include <QDesktopServices>
#include <QtSerialPort>
#include <QMessageBox>
#include <QtNetwork>
#include <QDebug>
#include <QMenu>


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    myPort = new serialport();
    this->on_refreshButton_clicked(); // 软件运行时自动扫描串口

    // 设置系统右下角托盘图标
    this->InitRightButtomIcon();
}

Widget::~Widget()
{
    qDebug()<< "close";
    delete ui;
    delete myPort;
    delete m_trayIcon;
    delete menu;
}


void Widget::InitRightButtomIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon(":/Icon/Icon/serialport.ico"));
    m_trayIcon->setToolTip(tr("串口助手"));    //设置鼠标放上去显示的信息

    menu = new QMenu(this);
    exitAction = new QAction(QIcon(":/Icon/Icon/exit.png"),tr("exit"),this);
    helpAction = new QAction(QIcon(":/Icon/Icon/help.png"),tr("help"),this);

    menu->addAction(helpAction);
    menu->addAction(exitAction);

    m_trayIcon->setContextMenu(menu);

    connect(helpAction, SIGNAL(triggered(bool)),this, SLOT(helpMe()));
    connect(exitAction, SIGNAL(triggered(bool)),this, SLOT(exitSerialPort()));
    connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(slotIconActivated(QSystemTrayIcon::ActivationReason)));
    m_trayIcon->show();
}

// 刷新端口和界面
void Widget::on_refreshButton_clicked()
{
    ui->portName->clear(); // 刷新前清空显示

    // 扫描电脑串口并显示
    portList = myPort->ScanSerialPort();
    for(int i=0;i<portList.count();i++){
        ui->portName->addItem(portList[i]);
    }

    // 清空界面输入
    ui->textEdit_read->setText("");
    ui->textEdit_write->setText("");
}

// 初始化串口
void Widget::on_pushButton_comopen_clicked()
{
    if(ui->portName->currentText() == ""){
        QMessageBox::warning(this,"打开失败","未找到串口！\n请检查串口是否插入...");
        return;
    }

    // 修改按钮文字及样式
    if(ui->pushButton_comopen->text() == "打开串口"){

        if(!myPort->GetPortStatu(ui->portName->currentText())){
            ui->textEdit_read->setText("[error]:该串口被其他程序占用...");
            QMessageBox::warning(this,"打开失败","串口被其他用户占用...");
            return;
        }

        ui->pushButton_send->setEnabled(true);
        ui->baudRate->setEnabled(false);
        ui->refreshButton->setEnabled(false);
        ui->pushButton_comopen->setText("关闭");
        ui->pushButton_comopen->setIcon(QIcon(":/Icon/Icon/stop.png"));
    }else{
        // 关闭串口
        if(myPort->PortIsOpen()){
            my_timer->stop();
            myPort->ClosePort();
        }

        ui->textEdit_read->setText("");
        ui->baudRate->setEnabled(true);
        ui->refreshButton->setEnabled(true);
        ui->pushButton_send->setEnabled(false);
        ui->pushButton_comopen->setText("打开串口");

        ui->ctrMotor->setText("打开");
        ui->ctrMotor->setIcon(QIcon(":/Icon/Icon/begin.png"));
        ui->fanController->setText("启动");
        ui->fanController->setIcon(QIcon(":/Icon/Icon/begin.png"));
        ui->pushButton_comopen->setIcon(QIcon(":/Icon/Icon/begin.png"));
        return;
    }

    // 初始化串口（端口，波特率）
    myPort->InitSerialPort(ui->portName->currentText(),ui->baudRate->currentText().toInt());

    // 设置定时器
    my_timer = new QTimer(this);
    connect( my_timer, SIGNAL( timeout() ), this, SLOT( readMyCom() ) );
    my_timer->start(1000);

    ui->textEdit_read->setText("打开成功！");
}


// 发送串口信息
void Widget::on_pushButton_send_clicked()
{

    if(!myPort->PortIsOpen()) return;

    myPort->SendMsgToPort(ui->textEdit_write->toPlainText().toUtf8().toHex());
}


// 读取串口信息
void Widget::readMyCom()
{
    requestData = myPort->ReadMsgForPort();

    if(requestData.isEmpty() ) return;

    // 电机控制反馈信息
    if( requestData[0] == 0x01 ){
        if(requestData[1] == 0x01){
            switch (requestData[3]) {
            case 0x4F:
                ui->rotaryDirection->setText("运行中...");
                break;
            case 0x43:
                ui->rotaryDirection->setText("已停止...");
                break;
            case 0x4C:
                ui->rotaryDirection->setText("反向转动...");
                break;
            case 0x52:
                ui->rotaryDirection->setText("正向转动...");
                break;
            default:
                break;
            }
        }
    }else{
        // 发送数据到App
        if(ui->startSocket->text() == "关闭服务")
            server->SendSocketMessage(requestData[0],requestData[1],requestData[2],requestData[3]);
    }

    // 按16进制显示
    if(ui->radioHexMsg->isChecked()){
        ui->textEdit_read->append("● " + ("0x"+requestData.toHex()));
    }

    // 安字符串形式显示
    if(ui->radioStrMsg->isChecked()){
        // 如果是命令则不显示
        if(requestData.length() == 4) return;

        ui->textEdit_read->append("● "+ QString::fromLocal8Bit( requestData ));
    }

    requestData.clear();
}

// 托盘菜单响应
void Widget::slotIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
        case QSystemTrayIcon::Trigger : // 鼠标单击
            setWindowState(Qt::WindowActive);
            activateWindow();
            break;
        default:
            break;
    }
}

// 拦截窗口关闭信号
void Widget::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton button;

    if(myPort->PortIsOpen()){
        button = QMessageBox::question(this, tr("退出程序"),
            QString(tr("警告：当前串口正在使用中，是否退出?")),
            QMessageBox::Yes | QMessageBox::No);
    }else{
        this->close();
    }

    if (button == QMessageBox::No) {
        event->ignore();  //忽略退出信号，程序继续运行
    }
    else if (button == QMessageBox::Yes) {
        this->close();
    }
}

// 退出应用
void Widget::exitSerialPort()
{
    this->close();
    //    exit(0);    // exit 退出不执行析构函数
}

// 帮助,使用默认浏览器打开页面
void Widget::helpMe()
{
    QDesktopServices::openUrl(QUrl("http://tan0510.blog.163.com/"));
}

// ============================================= 电机控制 ==========================
// 打开停止电机
void Widget::on_ctrMotor_clicked()
{
    if(!myPort->PortIsOpen()){
        QMessageBox::warning(this,"打开失败","请先打开串口！");
        return;
    }

    // 根据按钮状态修改提示信息
    if(ui->ctrMotor->text() == "打开"){
        ui->ctrMotor->setText("停止");
        ui->ctrMotor->setIcon(QIcon(":/Icon/Icon/stop.png"));
        myPort->Send_Com(0x01,0x4F); // "O"
    }else{
        ui->ctrMotor->setText("打开");
        ui->rotaryTime->setValue(1);
        ui->rotaryAnlge->setValue(0);
        myPort->Send_Com(0x01,0x43); // "C"
        ui->ctrMotor->setIcon(QIcon(":/Icon/Icon/begin.png"));
    }
}

// 脉冲时间
void Widget::on_rotaryTime_valueChanged(int ms)
{
    if(!myPort->PortIsOpen()) return;

    myPort->Send_TimeMs(ms);
}

// 控制旋转角度
void Widget::on_rotaryAnlge_valueChanged(int angle)
{
    if(!myPort->PortIsOpen()) return;

    myPort->Send_Data((angle/1.8)*2);
}

// 修改脉冲
void Widget::on_rotaryPulse_valueChanged(int pulse)
{
    if(!myPort->PortIsOpen()) return;

    myPort->Send_Data(pulse);
}

// 点击左转
void Widget::on_rotaryLeft_clicked()
{

    if(!myPort->PortIsOpen()){
        QMessageBox::warning(this,"打开失败","请先打开串口！");
        return;
    }

    if(ui->ctrMotor->text() == "打开"){
        QMessageBox::information(this,"提示","请打开控制！");
        return;
    }

    myPort->Send_Com(0x01,0x4C);  // "L"
}

// 点击右转
void Widget::on_rotaryRight_clicked()
{
    if(!myPort->PortIsOpen()){
        QMessageBox::warning(this,"打开失败","请先打开串口！");
        return;
    }

    if(ui->ctrMotor->text() == "打开"){
        QMessageBox::information(this,"提示","请打开控制！");
        return;
    }
    myPort->Send_Com(0x01,0x52);  // "R"
}

// 清空接收信息
void Widget::on_clearMsg_clicked()
{
    ui->textEdit_read->setText("");
}

// ============================================ 风扇控制 ====================
void Widget::on_fanController_clicked()
{
    if(!myPort->PortIsOpen()){
        QMessageBox::warning(this,"打开失败","请先打开串口！");
        return;
    }

    if(ui->fanController->text() == "启动" ){
        ui->fanController->setText("停止");
        ui->fanController->setIcon(QIcon(":/Icon/Icon/stop.png"));
        myPort->Send_Com(0x02,0x4F);
    }else{
        ui->fanController->setText("启动");
        ui->fanController->setIcon(QIcon(":/Icon/Icon/begin.png"));
        myPort->Send_Com(0x02,0x43);
    }
}

// 风扇 东
void Widget::on_fanEast_clicked()
{
    if(ui->fanController->text() == "启动"){
        QMessageBox::information(this,"提示","请先启动风扇！");
        return;
    }

    myPort->Send_Com(0x02,0x45);
}


// 风扇 南
void Widget::on_fanSouth_clicked()
{
    if(ui->fanController->text() == "启动"){
        QMessageBox::information(this,"提示","请先启动风扇！");
        return;
    }
    myPort->Send_Com(0x02,0x53);
}

// 风扇 西
void Widget::on_fanWest_clicked()
{
    if(ui->fanController->text() == "启动"){
        QMessageBox::information(this,"提示","请先启动风扇！");
        return;
    }
    myPort->Send_Com(0x02,0x57);
}

// 风扇 北
void Widget::on_fanNorth_clicked()
{
    if(ui->fanController->text() == "启动"){
        QMessageBox::information(this,"提示","请先启动风扇！");
        return;
    }
    myPort->Send_Com(0x02,0x4E);
}

// X 轴角度控制
void Widget::on_spinBox_X_valueChanged(int x_angle)
{
    myPort->Send_Angle(0x01,x_angle);
}

// Y 轴角度控制
void Widget::on_spinBox_Y_valueChanged(int y_angle)
{
     myPort->Send_Angle(0x02,y_angle);
}

// Z 轴角度控制
void Widget::on_spinBox_Z_valueChanged(int z_angle)
{
     myPort->Send_Angle(0x03,z_angle);
}

// 西北
void Widget::on_fanWestNorth_clicked()
{
    if(ui->fanController->text() == "启动"){
        QMessageBox::information(this,"提示","请先启动风扇！");
        return;
    }
    myPort->Send_Com(0x02,0xA5);
}

// 东北
void Widget::on_fanEastNorth_clicked()
{
    if(ui->fanController->text() == "启动"){
        QMessageBox::information(this,"提示","请先启动风扇！");
        return;
    }
    myPort->Send_Com(0x02,0x93);
}

// 东南
void Widget::on_fanEastSouth_clicked()
{
    if(ui->fanController->text() == "启动"){
        QMessageBox::information(this,"提示","请先启动风扇！");
        return;
    }
    myPort->Send_Com(0x02,0x98);
}

// 西南
void Widget::on_fanWestSouth_clicked()
{
    if(ui->fanController->text() == "启动"){
        QMessageBox::information(this,"提示","请先启动风扇！");
        return;
    }
    myPort->Send_Com(0x02,0xAA);
}

/* ====================================== Socket ============================ */
// 启动 Socket 服务
void Widget::on_startSocket_clicked()
{
    if(!myPort->PortIsOpen()){
        QMessageBox::warning(this,"打开失败","请先打开串口！");
        return;
    }

    if(ui->startSocket->text() == "启动服务"){
        ui->startSocket->setText("关闭服务");
        ui->startSocket->setIcon(QIcon(":/Icon/Icon/stop.png"));
    }else{
        if(server != NULL){
            server->closed();
            delete server;
        }
        ui->startSocket->setText("启动服务");
        ui->startSocket->setIcon(QIcon(":/Icon/Icon/begin.png"));
        return;
    }

    // 选择端口启动服务
    server = new socketserver(myPort, ui->socketPortName->currentText().toInt(), true);
}

// 获取本机IPV4地址
void Widget::on_refresIpv4_clicked()
{
     QList<QHostAddress> addList = QNetworkInterface::allAddresses();

     foreach(QHostAddress address,addList)
     {
         //排除IPV6，排除回环地址
         if(address.protocol() == QAbstractSocket::IPv4Protocol
                 && address != QHostAddress(QHostAddress::LocalHost))
         {
             //输出，转换为字符串格式
             ui->showIpv4->setText(address.toString());
         }
     }
}
