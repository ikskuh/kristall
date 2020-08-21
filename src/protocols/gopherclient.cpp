#include "gopherclient.hpp"
#include "ioutil.hpp"

GopherClient::GopherClient(QObject *parent) : ProtocolHandler(parent)
{
    connect(&socket, &QTcpSocket::connected, this, &GopherClient::on_connected);
    connect(&socket, &QTcpSocket::readyRead, this, &GopherClient::on_readRead);
    connect(&socket, &QTcpSocket::disconnected, this, &GopherClient::on_finished);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    connect(&socket, &QTcpSocket::errorOccurred, this, &GopherClient::on_socketError);
#else
    connect(&socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, &GopherClient::on_socketError);
#endif
}

GopherClient::~GopherClient()
{

}

bool GopherClient::supportsScheme(const QString &scheme) const
{
    return (scheme == "gopher");
}

bool GopherClient::startRequest(const QUrl &url, RequestOptions options)
{
    Q_UNUSED(options)

    if(isInProgress())
        return false;

    if(url.scheme() != "gopher")
        return false;

    // Second char on the URL path denotes the Gopher type
    // See https://tools.ietf.org/html/rfc4266
    QString type = url.path().mid(1, 1);

    mime = "application/octet-stream";
    if(type == "") mime = "text/gophermap";
    else if(type == "0") mime = "text/plain";
    else if(type == "1") mime = "text/gophermap";
    else if(type == "g") mime = "image/gif";
    else if(type == "I") mime = "image/unknown";
    else if(type == "h") mime = "text/html";
    else if(type == "s") mime = "audio/unknown";

    is_processing_binary = (type == "5") or (type == "9") or (type == "I") or (type == "g");

    this->requested_url = url;
    this->was_cancelled = false;
    socket.connectToHost(url.host(), url.port(70));

    return true;
}

bool GopherClient::isInProgress() const
{
    return socket.isOpen();
}

bool GopherClient::cancelRequest()
{
    was_cancelled = true;
    if (socket.state() != QTcpSocket::UnconnectedState)
    {
        socket.disconnectFromHost();
        socket.waitForDisconnected(1500);
    }
    socket.close();
    body.clear();
    return true;
}

void GopherClient::on_connected()
{
    auto blob = (requested_url.path().mid(2) + "\r\n").toUtf8();

    IoUtil::writeAll(socket, blob);
}

void GopherClient::on_readRead()
{
    body.append(socket.readAll());

    if(not is_processing_binary) {
        // Strip the "lone dot" from gopher data
        if(int index = body.indexOf("\r\n.\r\n"); index >= 0) {
            body.resize(index + 2);
            socket.close();
        }
    }

    if(not was_cancelled) {
        emit this->requestProgress(body.size());
    }
}

void GopherClient::on_finished()
{
    if(not was_cancelled)
    {
        this->on_readRead();
        emit this->requestComplete(this->body, mime);
        was_cancelled = true;
    }
    body.clear();
}

void GopherClient::on_socketError(QAbstractSocket::SocketError error_code)
{
    this->emitNetworkError(error_code, socket.errorString());
}
