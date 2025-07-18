#include "mainwindow.h"
#include <QApplication>


int main(int argc, char* argv[])
{
    
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    QFont defaultFont = QApplication::font();
    defaultFont.setPointSize(12);
    defaultFont.setFamily("Microsoft YaHei UI");
    a.setFont(defaultFont);

    MainWindow w;
    w.show();
    return a.exec();
}