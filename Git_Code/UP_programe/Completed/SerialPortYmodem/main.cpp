#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    //QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    return a.exec();
}
