#ifndef GEMINICLIENT_HPP
#define GEMINICLIENT_HPP

#include <QObject>
#include <QMimeType>
#include <QSslSocket>
#include <QUrl>

#include "cryptoidentity.hpp"

enum class TemporaryFailure {
    unspecified,
    server_unavailable,
    cgi_error,
    proxy_error,
    slow_down,
};

enum class PermanentFailure {
    unspecified,
    not_found,
    gone,
    proxy_request_required,
    bad_request,
};

enum class CertificateRejection {
    unspecified,
    not_accepted,
    future_certificate_rejected,
    expired_certificate_rejected,
};

class GeminiClient : public QObject
{
private:
    Q_OBJECT
public:
    explicit GeminiClient(QObject *parent = nullptr);

    ~GeminiClient() override;

    bool startRequest(QUrl const & url);

    bool isInProgress() const;

    bool cancelRequest();

    void enableClientCertificate(CryptoIdentity const & ident);
    void disableClientCertificate();

signals:
    void requestProgress(qint64 transferred);

    void requestComplete(QByteArray const & data, QString const & mime);

    void protocolViolation(QString const & reason);

    void inputRequired(QString const & query);

    void redirected(QUrl const & uri, bool is_permanent);

    void temporaryFailure(TemporaryFailure reason, QString const & info);

    void permanentFailure(PermanentFailure reason, QString const & info);

    void transientCertificateRequested(QString const & reason);

    void authorisedCertificateRequested(QString const & reason);

    void certificateRejected(CertificateRejection reason, QString const & info);

    void networkError(QString const & reason);

private slots:

    void socketEncrypted();

    void socketReadyRead();

    void socketDisconnected();

    void sslErrors(const QList<QSslError> &errors);

    void socketError(QAbstractSocket::SocketError socketError);


private:
    bool is_receiving_body;

    QUrl target_url;
    QSslSocket socket;
    QByteArray buffer;
    QByteArray body;
    QString mime_type;
};

#endif // GEMINICLIENT_HPP
