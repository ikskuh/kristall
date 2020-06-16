#include "geminiclient.hpp"
#include <cassert>
#include <QDebug>
#include <QSslConfiguration>
#include "kristall.hpp"

GeminiClient::GeminiClient() : ProtocolHandler(nullptr)
{
    connect(&socket, &QSslSocket::encrypted, this, &GeminiClient::socketEncrypted);
    connect(&socket, &QSslSocket::readyRead, this, &GeminiClient::socketReadyRead);
    connect(&socket, &QSslSocket::disconnected, this, &GeminiClient::socketDisconnected);
    connect(&socket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), this, &GeminiClient::sslErrors);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    connect(&socket, &QTcpSocket::errorOccurred, this, &GeminiClient::socketError);
#else
    connect(&socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, &GeminiClient::socketError);
#endif
}

GeminiClient::~GeminiClient()
{
    is_receiving_body = false;
}

bool GeminiClient::supportsScheme(const QString &scheme) const
{
    return (scheme == "gemini");
}

bool GeminiClient::startRequest(const QUrl &url)
{
    if(url.scheme() != "gemini")
        return false;

    if(socket.isOpen())
        return false;

    QSslConfiguration ssl_config;
    ssl_config.setProtocol(QSsl::TlsV1_2);
    if(not global_trust.enable_ca)
        ssl_config.setCaCertificates(QList<QSslCertificate> { });
    else
        ssl_config.setCaCertificates(QSslConfiguration::systemCaCertificates());
    socket.setSslConfiguration(ssl_config);

    socket.connectToHostEncrypted(url.host(), url.port(1965));

    buffer.clear();
    body.clear();
    is_receiving_body = false;

    if(not socket.isOpen())
        return false;

    target_url = url;
    mime_type = "<invalid>";

    return true;
}

bool GeminiClient::isInProgress() const
{
    return socket.isOpen();
}

bool GeminiClient::cancelRequest()
{
    this->is_receiving_body = false;
    this->socket.close();
    this->buffer.clear();
    this->body.clear();
    return true;
}

bool GeminiClient::enableClientCertificate(const CryptoIdentity &ident)
{
    this->socket.setLocalCertificate(ident.certificate);
    this->socket.setPrivateKey(ident.private_key);
    return true;
}

void GeminiClient::disableClientCertificate()
{
    this->socket.setLocalCertificate(QSslCertificate{});
    this->socket.setPrivateKey(QSslKey { });
}

void GeminiClient::socketEncrypted()
{
    qDebug() << "Pub key =" << socket.peerCertificate().publicKey().toPem();

    QString request = target_url.toString(QUrl::FormattingOptions(QUrl::FullyEncoded)) + "\r\n";

    QByteArray request_bytes = request.toUtf8();

    qint64 offset = 0;
    while(offset < request_bytes.size()) {
        auto const len = socket.write(request_bytes.constData() + offset, request_bytes.size() - offset);
        if(len <= 0)
        {
            socket.close();
            return;
        }
        offset += len;
    }
}

void GeminiClient::socketReadyRead()
{
    QByteArray response = socket.readAll();

    if(is_receiving_body)
    {
        body.append(response);
        emit this->requestProgress(body.size());
    }
    else
    {
        for(int i = 0; i < response.size(); i++)
        {
            if(response[i] == '\n') {
                buffer.append(response.data(), i);
                body.append(response.data() + i + 1, response.size() - i - 1);

                // "XY " <META> <CR> <LF>
                if(buffer.size() <= 5) {
                    socket.close();
                    qDebug() << buffer;
                    emit networkError(ProtocolViolation, "Line is too short for valid protocol");
                    return;
                }
                if(buffer[buffer.size() - 1] != '\r') {
                    socket.close();
                    qDebug() << buffer;
                    emit networkError(ProtocolViolation, "Line does not end with <CR> <LF>");
                    return;
                }
                if(not isdigit(buffer[0])) {
                    socket.close();
                    qDebug() << buffer;
                    emit networkError(ProtocolViolation, "First character is not a digit.");
                    return;
                }
                if(not isdigit(buffer[1])) {
                    socket.close();
                    qDebug() << buffer;
                    emit networkError(ProtocolViolation, "Second character is not a digit.");
                    return;
                }
                // TODO: Implement stricter version
                // if(buffer[2] != ' ') {
                if(not isspace(buffer[2])) {
                    socket.close();
                    qDebug() << buffer;
                    emit networkError(ProtocolViolation, "Third character is not a space.");
                    return;
                }

                QString meta = QString::fromUtf8(buffer.data() + 3, buffer.size() - 4);

                int primary_code = buffer[0] - '0';
                int secondary_code = buffer[1] - '0';

                qDebug() << primary_code << secondary_code << meta;

                // We don't need to receive any data after that.
                if(primary_code != 2)
                    socket.close();

                switch(primary_code)
                {
                case 1: // requesting input
                    emit inputRequired(meta);
                    return;

                case 2: // success
                    is_receiving_body = true;
                    mime_type = meta;
                    return;

                case 3: { // redirect
                    QUrl new_url(meta);
                    if(new_url.isValid()) {
                        if(new_url.isRelative())
                            new_url =  target_url.resolved(new_url);
                        assert(not new_url.isRelative());

                        emit redirected(new_url, (secondary_code == 1));
                    }
                    else {
                        emit networkError(ProtocolViolation, "Invalid URL for redirection!");
                    }
                    return;
                }

                case 4: { // temporary failure
                    NetworkError type = UnknownError;
                    switch(secondary_code)
                    {
                    case 1: type = InternalServerError; break;
                    case 2: type = InternalServerError; break;
                    case 3: type = InternalServerError; break;
                    case 4: type = UnknownError; break;
                    }
                    emit networkError(type, meta);
                    return;
                }

                case 5: { // permanent failure
                    NetworkError type = UnknownError;
                    switch(secondary_code)
                    {
                    case 1: type = ResourceNotFound; break;
                    case 2: type = ResourceNotFound; break;
                    case 3: type = BadRequest; break;
                    case 9: type = BadRequest; break;
                    }
                    emit networkError(type, meta);
                    return;
                }

                case 6: // client certificate required
                    switch(secondary_code)
                    {
                    case 0:
                        emit certificateRequired(meta);
                        return;

                    case 1:
                        emit networkError(Unauthorized, meta);
                        return;

                    default:
                    case 2:
                        emit networkError(InvalidClientCertificate, meta);
                        return;
                    }
                    return;

                default:
                    emit networkError(ProtocolViolation, "Unspecified status code used!");
                    return;
                }

                assert(false and "unreachable");
            }
        }
        buffer.append(response);
    }
}

void GeminiClient::socketDisconnected()
{
    if(is_receiving_body) {
        body.append(socket.readAll());
        emit requestComplete(body, mime_type);
    }
}

static bool isTrustRelated(QSslError::SslError err)
{
    switch(err)
    {
    case QSslError::CertificateUntrusted: return true;
    case QSslError::SelfSignedCertificate: return true;
    case QSslError::UnableToGetLocalIssuerCertificate: return true;
    default: return false;
    }
}

void GeminiClient::sslErrors(QList<QSslError> const & errors)
{
    QList<QSslError> remaining_errors = errors;
    QList<QSslError> ignored_errors;

    int i = 0;
    while(i < remaining_errors.size())
    {
        auto const & err = remaining_errors.at(i);

        bool ignore = false;
        if(isTrustRelated(err.error()))
        {
            if(global_trust.isTrusted(target_url, socket.peerCertificate()))
            {
                ignore = true;
            }
            else
            {
                emit this->networkError(UntrustedHost, "The requested host is not trusted.");
                return;
            }
        }
        else if(err.error() == QSslError::UnableToVerifyFirstCertificate)
        {
            ignore = true;
        }

        if(ignore) {
            ignored_errors.append(err);
            remaining_errors.removeAt(0);
        } else {
            i += 1;
        }
    }

    socket.ignoreSslErrors(ignored_errors);

    qDebug() << "ignoring" << ignored_errors.size() << "out of" << errors.size();

    for(auto const & error : remaining_errors) {
        qWarning() << int(error.error()) << error.errorString();
    }

    if(remaining_errors.size() > 0) {
        emit this->networkError(TlsFailure, remaining_errors.first().errorString());
    }
}

void GeminiClient::socketError(QAbstractSocket::SocketError socketError)
{
    // When remote host closes TLS session, the client closes the socket.
    // This is more sane then erroring out here as it's a perfectly legal
    // state and we know the TLS connection has ended.
    if(socketError == QAbstractSocket::RemoteHostClosedError) {
        socket.close();
    } else {
        this->emitNetworkError(socketError, socket.errorString());
    }
}
