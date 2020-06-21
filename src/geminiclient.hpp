#ifndef GEMINICLIENT_HPP
#define GEMINICLIENT_HPP

#include <QObject>
#include <QMimeType>
#include <QSslSocket>
#include <QUrl>

#include "protocolhandler.hpp"

class GeminiClient : public ProtocolHandler
{
private:
    Q_OBJECT
public:
    explicit GeminiClient();

    ~GeminiClient() override;

    bool supportsScheme(QString const & scheme) const override;

    bool startRequest(QUrl const & url, RequestOptions options) override;

    bool isInProgress() const override;

    bool cancelRequest() override;

    bool enableClientCertificate(CryptoIdentity const & ident) override;
    void disableClientCertificate() override;

private slots:
    void socketEncrypted();

    void socketReadyRead();

    void socketDisconnected();

    void sslErrors(const QList<QSslError> &errors);

    void socketError(QAbstractSocket::SocketError socketError);

private:
    bool is_receiving_body;
    bool suppress_socket_tls_error;

    QUrl target_url;
    QSslSocket socket;
    QByteArray buffer;
    QByteArray body;
    QString mime_type;
    RequestOptions options;
};

#endif // GEMINICLIENT_HPP
