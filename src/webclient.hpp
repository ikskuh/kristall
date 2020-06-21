#ifndef WEBCLIENT_HPP
#define WEBCLIENT_HPP

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "protocolhandler.hpp"

class WebClient: public ProtocolHandler
{
private:
    Q_OBJECT
public:
    explicit WebClient();

    ~WebClient() override;

    bool supportsScheme(QString const & scheme) const override;

    bool startRequest(QUrl const & url, RequestOptions options) override;

    bool isInProgress() const override;

    bool cancelRequest() override;

    bool enableClientCertificate(CryptoIdentity const & ident) override;
    void disableClientCertificate() override;

private slots:
    void on_data();
    void on_finished();
    void on_sslErrors(const QList<QSslError> &errors);
    void on_redirected(const QUrl &url);

private:
    QNetworkAccessManager manager;
    QNetworkReply * current_reply;

    QByteArray body;
    RequestOptions options;

    CryptoIdentity current_identity;

    bool suppress_socket_tls_error;
};

#endif // WEBCLIENT_HPP
