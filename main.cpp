#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.setWindowTitle("串口调试助手");
    w.setFixedSize(750,450);
    w.show();

    return a.exec();
}
