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

void ProtocolHandler::emitNetworkError(QAbstractSocket::SocketError error_code, const QString &textual_description)
{
    NetworkError network_error = UnknownError;
    switch (error_code)
    {
    case QAbstractSocket::ConnectionRefusedError:
        network_error = ConnectionRefused;
        break;
    case QAbstractSocket::HostNotFoundError:
        network_error = HostNotFound;
        break;
    case QAbstractSocket::SocketTimeoutError:
        network_error = Timeout;
        break;
    case QAbstractSocket::SslHandshakeFailedError:
        network_error = TlsFailure;
        break;
    case QAbstractSocket::SslInternalError:
        network_error = TlsFailure;
        break;
    case QAbstractSocket::SslInvalidUserDataError:
        network_error = TlsFailure;
        break;
    default:
        qDebug() << "unhandled network error:" << error_code;
        break;
    }
    emit this->networkError(network_error, textual_description);
}
