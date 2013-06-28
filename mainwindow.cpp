// mainwindow.cpp Copyright 2013 Owen Lynn <owen.lynn@gmail.com>
// Release under the GNU Public License V3

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "uicardnetbroker.h"
#include <QMessageBox>

TerminalMainWindow::TerminalMainWindow(QWidget *parent) :
    QMainWindow(parent), m_ui(new Ui::TerminalMainWindow)
{
    m_ui->setupUi(this);
}

TerminalMainWindow::~TerminalMainWindow()
{
    delete m_ui;
}

void
TerminalMainWindow::SetUIBroker(UICardNetBroker *broker)
{
    m_broker = broker;
}

void
TerminalMainWindow::onGetAccountDetails()
{
    QString pin = m_ui->pin_edit->text();
    QString recv_addr,acct_balance,txfee;
    int rc = m_broker->GetAccountDetails(pin,recv_addr,acct_balance,txfee);
    if (rc < 0) {
        QString fail_msg;
        fail_msg += "Failure code = ";
        fail_msg += "0x" + QByteArray::number(rc,16);
        QMessageBox::critical(this,"Get Account Details", fail_msg);
    } else {
        QMessageBox::information(this,"Get Account Details", "Success!");
        m_ui->recv_addr_disp->setText(recv_addr);
        m_ui->acct_balance_disp->setText(acct_balance);
        m_ui->txfee_disp->setText(txfee);
    }
}

void
TerminalMainWindow::onMakePayment()
{
    QString pin = m_ui->pin_edit->text();
    QString amount = m_ui->amount_edit->text();
    QString payto_addr = m_ui->payto_addr_edit->text();
    QString txid_out;
    // TODO: adde a base58 decode check here to save upstream some time
    int rc = m_broker->MakePayment(pin,amount,payto_addr,txid_out);
    if (rc < 0) {
        QString fail_msg;
        fail_msg += "Failure code = ";
        fail_msg += "0x" + QByteArray::number(rc,16);
        QMessageBox::critical(this,"Get Account Details", fail_msg);
    } else {
        QString success_msg = "Success, TXID = " + txid_out;
        QMessageBox::information(this,"Get Account Details", success_msg);
    }
}
