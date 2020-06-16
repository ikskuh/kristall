#include "protocolhandler.hpp"

ProtocolHandler::ProtocolHandler(QObject *parent) : QObject(parent)
{

}

bool ProtocolHandler::enableClientCertificate(const CryptoIdentity &ident)
{
    Q_UNUSED(ident);
    return true;
}

void ProtocolHandler::disableClientCertificate()
{

}
