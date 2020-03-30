#include "mainwindow.h"

#include <QApplication>
#include <QString>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString arg(argv[1]);
    MainWindow w(arg);
    w.show();
    return a.exec();
}
