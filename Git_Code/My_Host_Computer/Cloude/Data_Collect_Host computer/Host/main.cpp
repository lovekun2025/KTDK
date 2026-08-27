//#pragma execution_character_set("utf-8")
#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));
    a.setApplicationName("数据采集上位机");
    a.setOrganizationName("Host");

    MainWindow w;
    w.show();
    return a.exec();
}
