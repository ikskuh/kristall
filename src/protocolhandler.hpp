#ifndef GENERICPROTOCOLCLIENT_HPP
#define GENERICPROTOCOLCLIENT_HPP

#include <QObject>

#include "cryptoidentity.hpp"

class ProtocolHandler : public QObject
{
    Q_OBJECT
public:
    enum NetworkError {
        UnknownError, //!< There was an unhandled network error
        ProtocolViolation, //!< The server responded with something unexpected and violated the protocol
        HostNotFound, //!< The host
        ResourceNotFound, //!< The requested resource was not found on the server
        BadRequest, //!< Our client misbehaved and did a request the server cannot understand
        ProxyRequest, //!< We requested to
        InternalServerError,
        InvalidClientCertificate,
        UntrustedHost, //!< We don't know the host, and we don't trust it
        MistrustedHost, //!< We know the host and it's not the server identity we've seen before
        Unauthorized, //!< The requested resource could not be accessed.
        TlsFailure, //!< Unspecified TLS failure
    };
public:
    explicit ProtocolHandler(QObject *parent = nullptr);

    virtual bool supportsScheme(QString const & scheme) const = 0;

    virtual bool startRequest(QUrl const & url) = 0;

    virtual bool isInProgress() const = 0;

    virtual bool cancelRequest() = 0;

    virtual bool enableClientCertificate(CryptoIdentity const & ident);
    virtual void disableClientCertificate();
signals:
    //! We successfully transferred some bytes from the server
    void requestProgress(qint64 transferred);

    //! The request completed with the given data and mime type
    void requestComplete(QByteArray const & data, QString const & mime);

    //! Server redirected us to another URL
    void redirected(QUrl const & uri, bool is_permanent);

    //! The server needs some information from the user to process this query.
    void inputRequired(QString const & user_query);

    //! There was an error while processing the request
    void networkError(NetworkError error, QString const & reason);

    //! The server wants us to use a client certificate
    void certificateRequired(QString const & info);
};

#endif // GENERICPROTOCOLCLIENT_HPP
