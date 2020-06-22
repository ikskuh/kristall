#ifndef GOPHERCLIENT_HPP
#define GOPHERCLIENT_HPP

#include <QObject>
#include <QTcpSocket>
#include <QUrl>

#include "protocolhandler.hpp"

class GopherClient : public ProtocolHandler
{
    Q_OBJECT
public:
    explicit GopherClient(QObject *parent = nullptr);

    ~GopherClient() override;

    bool supportsScheme(QString const & scheme) const override;

    bool startRequest(QUrl const & url, RequestOptions options) override;

    bool isInProgress() const override;

    bool cancelRequest() override;

private: // slots
    void on_connected();
    void on_readRead();
    void on_finished();
    void on_socketError(QAbstractSocket::SocketError errorCode);


private:
    QTcpSocket socket;
    QByteArray body;
    QUrl requested_url;
    bool was_cancelled;
    QString mime;
    bool is_processing_binary;
};

#endif // GOPHERCLIENT_HPP
