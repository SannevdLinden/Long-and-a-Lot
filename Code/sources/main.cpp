#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setStyleSheet("background-color:rgb(48,48,48); border: 0px; color:white; "
                    "font:Helvetica;");

    //w.showMaximized();
    w.show();
    w.init();
    //w.show();
    return a.exec();
}
