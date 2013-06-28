// mainwindow.h Copyright 2013 Owen Lynn <owen.lynn@gmail.com>
// Released under the GNU Public License V3

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class UICardNetBroker;

namespace Ui {
class TerminalMainWindow;
}

class TerminalMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit TerminalMainWindow(QWidget *parent = 0);
    ~TerminalMainWindow();

    void SetUIBroker(UICardNetBroker *broker);

public slots:
    void onGetAccountDetails();
    void onMakePayment();
    
private:
    Ui::TerminalMainWindow *m_ui;
    UICardNetBroker        *m_broker;
};

#endif // MAINWINDOW_H
