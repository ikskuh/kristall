#ifndef FINGERCLIENT_HPP
#define FINGERCLIENT_HPP

#include <QObject>
#include <QTcpSocket>
#include <QUrl>

#include "protocolhandler.hpp"

class FingerClient : public ProtocolHandler
{
    Q_OBJECT
public:
    explicit FingerClient();

    ~FingerClient() override;

    bool supportsScheme(QString const & scheme) const override;

    bool startRequest(QUrl const & url, RequestOptions options) override;

    bool isInProgress() const override;

    bool cancelRequest() override;

private slots:
    void on_connected();
    void on_readRead();
    void on_finished();
    void on_socketError(QTcpSocket::SocketError error_code);

private:
    QTcpSocket socket;
    QByteArray body;
    bool was_cancelled;
    QString requested_user;
};

#endif // FINGERCLIENT_HPP
