#include "fingerclient.hpp"
#include "ioutil.hpp"
#include "kristall.hpp"

FingerClient::FingerClient() : ProtocolHandler(nullptr)
{
    connect(&socket, &QTcpSocket::connected, this, &FingerClient::on_connected);
    connect(&socket, &QTcpSocket::readyRead, this, &FingerClient::on_readRead);
    connect(&socket, &QTcpSocket::disconnected, this, &FingerClient::on_finished);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    connect(&socket, &QTcpSocket::errorOccurred, this, &FingerClient::on_socketError);
#else
    connect(&socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, &FingerClient::on_socketError);
#endif

    connect(&socket, &QAbstractSocket::hostFound, this, [this]() {
        emit this->requestStateChange(RequestState::HostFound);
    });
    emit this->requestStateChange(RequestState::None);
}

FingerClient::~FingerClient()
{

}

bool FingerClient::supportsScheme(const QString &scheme) const
{
    return (scheme == "finger");
}

bool FingerClient::startRequest(const QUrl &url, RequestOptions options)
{
    Q_UNUSED(options)

    if(isInProgress())
        return false;

    if(url.scheme() != "finger")
        return false;

    this->requested_user = url.userName();
    this->was_cancelled = false;
    socket.connectToHost(url.host(), url.port(79));

    return true;
}

bool FingerClient::isInProgress() const
{
    return socket.isOpen();
}

bool FingerClient::cancelRequest()
{
    was_cancelled = true;
    if (socket.state() != QTcpSocket::UnconnectedState)
    {
        socket.disconnectFromHost();
        this->socket.waitForDisconnected(500);
    }
    socket.close();
    body.clear();
    return true;
}

void FingerClient::on_connected()
{
    auto blob = (requested_user + "\r\n").toUtf8();

    IoUtil::writeAll(socket, blob);

    emit this->requestStateChange(RequestState::Connected);
}

void FingerClient::on_readRead()
{
    body.append(socket.readAll());
    emit this->requestProgress(body.size());
}

void FingerClient::on_finished()
{
    if(not was_cancelled)
    {
        emit this->requestComplete(this->body, "text/finger");
        was_cancelled = true;
    }
    body.clear();

    emit this->requestStateChange(RequestState::None);
}

void FingerClient::on_socketError(QAbstractSocket::SocketError error_code)
{
    this->emitNetworkError(error_code, socket.errorString());
}
