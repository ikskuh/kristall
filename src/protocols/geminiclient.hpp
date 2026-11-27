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

    bool startUpload(QUrl const & url, QByteArray const & data,
                     QString const & mime, QString const & token,
                     RequestOptions options) override;

    bool isUploadScheme(QString const & scheme) const override;

    bool isEditUrl(QUrl const & url) const override;

    QUrl viewUrl(QUrl const & url) const override;

    bool isInProgress() const override;

    bool cancelRequest() override;

    bool enableClientCertificate(CryptoIdentity const & ident) override;
    void disableClientCertificate() override;

private slots:
    void socketEncrypted();

    void socketReadyRead();

    void socketBytesWritten(qint64 bytes);

    void socketDisconnected();

    void sslErrors(const QList<QSslError> &errors);

    void socketError(QAbstractSocket::SocketError socketError);

private:
    static QUrl stripTitanParameters(QUrl url);

    bool openConnection(QUrl const & url, RequestOptions options);

private:
    bool is_receiving_body;
    bool suppress_socket_tls_error;
    bool is_error_state;

    QUrl target_url;
    QSslSocket socket;
    QByteArray buffer;
    QByteArray body;
    QByteArray upload_data;
    qint64 bytes_sent;
    QString mime_type;
    RequestOptions options;
};

#endif // GEMINICLIENT_HPP
