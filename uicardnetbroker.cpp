// UICardNetBroker.cpp Copyright 2013 Owen Lynn <owen.lynn@gmail.com>
// Release under the GNU Public License V3

#include "uicardnetbroker.h"
#include "SmartCardMarlene.h"
#include "SocketClient.h"
#include "Util.h"

#define SUCCESS                                                    0x00000000
#define ERROR_UNKNOWN                                              0x80000000
#define ERROR_GETACCOUNTDETAILS_CARD_INIT_FAIL                     0x80000001
#define ERROR_GETACCOUNTDETAILS_CARD_WAITCONNECT_FAIL              0x80000002
#define ERROR_GETACCOUNTDETAILS_CARD_SAYHELLO_FAIL                 0x80000003
#define ERROR_GETACCOUNTDETAILS_CARD_GETREQBUFFER_FAIL             0x80000004
#define ERROR_GETACCOUNTDETAILS_CARD_GETHOSTDATA_FAIL              0x80000005
#define ERROR_GETACCOUNTDETAILS_SOCKET_CONNECT_FAIL                0x80000006
#define ERROR_GETACCOUNTDETAILS_SOCKET_TALKTOSERVER_FAIL           0x80000007
#define ERROR_GETACCOUNTDETAILS_CARD_POKESERVERRESP_FAIL           0x80000008
#define ERROR_GETACCOUNTDETAILS_CARD_CHECKERRORCODE_FAIL           0x80000009
#define ERROR_GETACCOUNTDETAILS_CARD_GETBALANCE_FAIL               0x8000000a
#define ERROR_GETACCOUNTDETAILS_CARD_GETTXFEE_FAIL                 0x8000000b
#define ERROR_GETACCOUNTDETAILS_CARD_GETRECVADDR_FAIL              0x8000000c
#define ERROR_MAKEPAYMENT_ZERO_AMOUNT                              0x8000000d
#define ERROR_MAKEPAYMENT_CARD_INIT_FAIL                           0x8000000e
#define ERROR_MAKEPAYMENT_CARD_WAITCONNECT_FAIL                    0x8000000f
#define ERROR_MAKEPAYMENT_CARD_SAYHELLO_FAIL                       0x80000010
#define ERROR_MAKEPAYMENT_CARD_GETREQBUFFER_FAIL                   0x80000011
#define ERROR_MAKEPAYMENT_CARD_GETHOSTDATA_FAIL                    0x80000012
#define ERROR_MAKEPAYMENT_SOCKET_CONNECT_FAIL                      0x80000013
#define ERROR_MAKEPAYMENT_SOCKET_TALKTOSERVER_FAIL                 0x80000014
#define ERROR_MAKEPAYMENT_CARD_POKESERVERRESP_FAIL                 0x80000015
#define ERROR_MAKEPAYMENT_CARD_CHECKERRORCODE_FAIL                 0x80000016
#define ERROR_MAKEPAYMENT_CARD_MAKEPAYMENT_FAIL                    0x80000017
#define ERROR_MAKEPAYMENT_CARD_GETPAYMENTSTATUS_FAIL               0x80000018


UICardNetBroker::UICardNetBroker()
{
}

UICardNetBroker::~UICardNetBroker()
{
}

int
UICardNetBroker::GetAccountDetails(const QString &qpin, QString &recv_addr, QString &acct_balance, QString &txfee_str)
{
    int rc = ERROR_UNKNOWN;
    SocketClient cli;
    SmartCardMarlene card;
    std::string pin,hostname,cardreq,resp,bcaddr_out;
    unsigned long port, server_status;
    unsigned long long balance, txfee;

    if (!card.Init()) {
        return ERROR_GETACCOUNTDETAILS_CARD_INIT_FAIL;
    }
    if (!card.WaitConnect()) {
        return ERROR_GETACCOUNTDETAILS_CARD_WAITCONNECT_FAIL;
    }


    pin = qpin.toStdString();
    if (!card.SayHello(pin)) {
        rc = ERROR_GETACCOUNTDETAILS_CARD_SAYHELLO_FAIL;
        goto error;
    }

    if (!GetCardRequestBuffer(pin,card,cardreq)) {
        rc = ERROR_GETACCOUNTDETAILS_CARD_GETREQBUFFER_FAIL;
        goto error;
    }

    if (!card.GetHostData(port,hostname)) {
        rc = ERROR_GETACCOUNTDETAILS_CARD_GETHOSTDATA_FAIL;
        goto error;
    }

    if (!cli.connect(hostname,port)) {
        rc = ERROR_GETACCOUNTDETAILS_SOCKET_CONNECT_FAIL;
        goto error;
    }

    if (!TalkToServer(cli,cardreq,resp)) {
        rc = ERROR_GETACCOUNTDETAILS_SOCKET_TALKTOSERVER_FAIL;
        goto error;
    }

    if (!PokeServerResponse(pin,card,resp)) {
        rc = ERROR_GETACCOUNTDETAILS_CARD_POKESERVERRESP_FAIL;
        goto error;
    }

    if (!card.CheckErrorCode(pin,server_status)) {
        rc = ERROR_GETACCOUNTDETAILS_CARD_CHECKERRORCODE_FAIL;
        goto error;
    }
    if (server_status < 0) {
        rc = server_status;
        goto error;
    }

    if (!card.GetBalance(pin,balance)) {
        rc = ERROR_GETACCOUNTDETAILS_CARD_GETBALANCE_FAIL;
        goto error;
    }
    if (!card.GetTxFee(pin,txfee)) {
        rc = ERROR_GETACCOUNTDETAILS_CARD_GETTXFEE_FAIL;
        goto error;
    }
    if (!card.GetReceivingAddress(pin,bcaddr_out)) {
        rc = ERROR_GETACCOUNTDETAILS_CARD_GETRECVADDR_FAIL;
        goto error;
    }

    recv_addr = QByteArray(bcaddr_out.c_str(),bcaddr_out.length());
    acct_balance = QByteArray::number(Utility::FromSatoshis(balance));
    txfee_str = QByteArray::number(Utility::FromSatoshis(txfee));

    // don't check these calls, if they fail the server will clean itself up
    card.SayGoodbye(pin);
    GetCardRequestBuffer(pin,card,cardreq);
    TalkToServer(cli,cardreq,resp);
    // don't bother processing the response either

    cli.close();
    card.Disconnect();
    card.Term();

    return SUCCESS;

error:
    cli.close();
    card.Disconnect();
    card.Term();

    return rc;
}

int
UICardNetBroker::MakePayment(const QString &qpin, QString &amount, QString &payto_addr, QString &txid_out)
{
    int rc = ERROR_UNKNOWN;
    SocketClient cli;
    SmartCardMarlene card;
    std::string pin,hostname,cardreq,resp,stl_txid;
    QByteArray qb_txid;
    unsigned long port, server_status,payment_status;
    unsigned long long amt_satoshis;
    double amt_double;

    amt_double = amount.toDouble();
    if (amt_double == 0.0) {
        return ERROR_MAKEPAYMENT_ZERO_AMOUNT;
    }
    amt_satoshis = Utility::ToSatoshis(amt_double);

    if (!card.Init()) {
        return ERROR_MAKEPAYMENT_CARD_INIT_FAIL;
    }
    if (!card.WaitConnect()) {
        return ERROR_MAKEPAYMENT_CARD_WAITCONNECT_FAIL;
    }


    pin = qpin.toStdString();
    if (!card.SayHello(pin)) {
        rc = ERROR_MAKEPAYMENT_CARD_SAYHELLO_FAIL;
        goto error;
    }

    if (!GetCardRequestBuffer(pin,card,cardreq)) {
        rc = ERROR_MAKEPAYMENT_CARD_GETREQBUFFER_FAIL;
        goto error;
    }

    if (!card.GetHostData(port,hostname)) {
        rc = ERROR_MAKEPAYMENT_CARD_GETHOSTDATA_FAIL;
        goto error;
    }

    if (!cli.connect(hostname,port)) {
        rc = ERROR_MAKEPAYMENT_SOCKET_CONNECT_FAIL;
        goto error;
    }

    if (!TalkToServer(cli,cardreq,resp)) {
        rc = ERROR_MAKEPAYMENT_SOCKET_TALKTOSERVER_FAIL;
        goto error;
    }

    if (!PokeServerResponse(pin,card,resp)) {
        rc = ERROR_MAKEPAYMENT_CARD_POKESERVERRESP_FAIL;
        goto error;
    }

    if (!card.CheckErrorCode(pin,server_status)) {
        rc = ERROR_MAKEPAYMENT_CARD_CHECKERRORCODE_FAIL;
        goto error;
    }
    if (server_status < 0) {
        rc = server_status;
        goto error;
    }

    if (!card.MakePayment(pin,amt_satoshis,payto_addr.toStdString())) {
        rc = ERROR_MAKEPAYMENT_CARD_MAKEPAYMENT_FAIL;
        goto error;
    }
    if (!GetCardRequestBuffer(pin,card,cardreq)) {
        rc = ERROR_MAKEPAYMENT_CARD_GETREQBUFFER_FAIL;
        goto error;
    }
    if (!TalkToServer(cli,cardreq,resp)) {
        rc = ERROR_MAKEPAYMENT_SOCKET_TALKTOSERVER_FAIL;
        goto error;
    }
    if (!PokeServerResponse(pin,card,resp)) {
        rc = ERROR_MAKEPAYMENT_CARD_POKESERVERRESP_FAIL;
        goto error;
    }
    if (!card.CheckErrorCode(pin,server_status)) {
        rc = ERROR_MAKEPAYMENT_CARD_CHECKERRORCODE_FAIL;
        goto error;
    }
    if (server_status < 0) {
        rc = server_status;
        goto error;
    }
    if (!card.GetPaymentStatus(pin,payment_status,stl_txid)) {
        rc = ERROR_MAKEPAYMENT_CARD_GETPAYMENTSTATUS_FAIL;
        goto error;
    }
    if (payment_status <0) {
        rc = payment_status;
        goto error;
    }
    qb_txid.clear();
    qb_txid.append(stl_txid.c_str(),stl_txid.length());
    txid_out = qb_txid.toHex();

    cli.close();
    card.Disconnect();
    card.Term();

    return SUCCESS;

error:
    cli.close();
    card.Disconnect();
    card.Term();

    return rc;
}

bool
UICardNetBroker::GetCardRequestBuffer(const std::string &pin, SmartCardMarlene &card, std::string &out)
{
    bool rc = false;
    unsigned long reqlen;
    rc = card.GetCardRequestLength(pin,reqlen);
    if (rc) {
        out.clear();
        unsigned long pos=1;
        while (rc && pos < reqlen) {
            std::string chunk;
            rc = card.ReadCardRequest(pin,pos,chunk);
            if (rc) {
                out += chunk;
                pos += chunk.length();
            }
        }
    }
    return rc;
}

bool
UICardNetBroker::TalkToServer(SocketClient &cli, const std::string &req, std::string &resp)
{
    if (!cli.write(req)) { return false; }

    unsigned short packetlen = 0;
    unsigned short readlen = sizeof(packetlen);
    if (!cli.read(&packetlen, readlen) || readlen != sizeof(packetlen)) { return false; }

    long checksum1; // note this is signed, has to be because BasicCard only has signed longs
    readlen = sizeof(checksum1);
    if (!cli.read(&checksum1,readlen) || readlen != sizeof(checksum1)) { return false; }

    unsigned char chunk[1024];
    if (packetlen > sizeof(chunk)) { return false; }
    readlen = packetlen;
    if (!cli.read((char*)chunk,readlen) || readlen != packetlen) { return false; }

    long checksum2;
    std::string payload;
    payload.append((char *)chunk,packetlen);
    checksum2 = Utility::CheckSum(payload);
    if (checksum1 == checksum2) {
        resp.clear();
        resp.append((char *)&packetlen, sizeof(packetlen));
        resp.append((char *)&checksum1, sizeof(checksum1));
        resp.append(payload);
        return true;
    }

    return false;
}

bool
UICardNetBroker::PokeServerResponse(const std::string &pin, SmartCardMarlene &card, const std::string &resp)
{
    bool rc=false;
    bool append=false;
    unsigned int i=0;
    while (i < resp.length()) {
        std::string chunk;
        chunk = resp.substr(i,SmartCardMarlene::SAFE_CHUNK_LENGTH);
        rc = card.SetServerResponse(pin,append,chunk);
        if (!rc) { break; }
        if (!append) { append = true; }
        i += chunk.length();
    }
    if (rc) {
        rc = card.ParseServerResponse(pin);
    }
    return rc;
}
