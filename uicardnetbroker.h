// UICardNetBroker.h Copyright 2013 Owen Lynn <owen.lynn@gmail.com>
// Released under the GNU Public License V3

#ifndef UICARDNETBROKER_H
#define UICARDNETBROKER_H
#include <QtCore>

class SmartCardMarlene;
class SocketClient;

class UICardNetBroker
{
public:
    UICardNetBroker();
    virtual ~UICardNetBroker();

    int GetAccountDetails(const QString &pin,QString &recv_addr,QString &acct_balance,QString &txfee);
    int MakePayment(const QString &pin, QString &amount, QString &payto_addr, QString &txid_out);

private:
    // don't imple these two
    UICardNetBroker(const UICardNetBroker &src);
    UICardNetBroker &operator= (const UICardNetBroker &src);

    bool GetCardRequestBuffer(const std::string &pin, SmartCardMarlene &card, std::string &out);
    bool TalkToServer(SocketClient &cli, const std::string &req, std::string &resp);
    bool PokeServerResponse(const std::string &pin, SmartCardMarlene &card, const std::string &resp);
};

#endif // UICARDNETBROKER_H
