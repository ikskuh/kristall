#ifndef GUPPYCLIENT_HPP
#define GUPPYCLIENT_HPP

#include <QObject>
#include <QUdpSocket>
#include <QUrl>
#include <QTimer>

#include "protocolhandler.hpp"

class GuppyClient : public ProtocolHandler
{
    Q_OBJECT
public:
    explicit GuppyClient(QObject *parent = nullptr);

    ~GuppyClient() override;

    bool supportsScheme(QString const & scheme) const override;

    bool startRequest(QUrl const & url, RequestOptions options) override;

    bool isInProgress() const override;

    bool cancelRequest() override;

private: // slots
    void on_connected();
    void on_readRead();
    void on_finished();
    void on_timerTick();
    void on_socketError(QAbstractSocket::SocketError errorCode);


private:
    QUdpSocket socket;
    QHash<long, QByteArray> chunks;
    QByteArray body;
    QUrl requested_url;
    QByteArray request;
    bool was_cancelled;
    int prev_seq, first_seq, last_seq;
    QString mime;
    QTimer timer;
};

#endif // GUPPYCLIENT_HPP
