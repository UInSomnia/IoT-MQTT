#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    MainWindow w;
    w.setWindowTitle(
        "IoT MQTT Client "
        "by InSomnia and Soni");
    w.showMaximized();
    
    return a.exec();
}
