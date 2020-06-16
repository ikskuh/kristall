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

    bool startRequest(QUrl const & url) override;

    bool isInProgress() const override;

    bool cancelRequest() override;

private slots:
    void on_connected();
    void on_readRead();
    void on_finished();

private:
    QTcpSocket socket;
    QByteArray body;
    bool was_cancelled;
    QString requested_user;
};

#endif // FINGERCLIENT_HPP
