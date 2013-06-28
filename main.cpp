// main.cpp Copyright 2013 Owen Lynn <owen.lynn@gmail.com>
// Release under the GNU Public License V3

#include "mainwindow.h"
#include "uicardnetbroker.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UICardNetBroker broker;
    TerminalMainWindow w;
    w.SetUIBroker(&broker);
    w.show();
    return a.exec();
}
