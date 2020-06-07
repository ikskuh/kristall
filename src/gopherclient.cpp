#include "gopherclient.hpp"
#include "ioutil.hpp"

GopherClient::GopherClient(QObject *parent) : QObject(parent)
{
    connect(&socket, &QTcpSocket::connected, this, &GopherClient::on_connected);
    connect(&socket, &QTcpSocket::readyRead, this, &GopherClient::on_readRead);
    connect(&socket, &QTcpSocket::disconnected, this, &GopherClient::on_finished);
}

GopherClient::~GopherClient()
{

}

bool GopherClient::startRequest(const QUrl &url)
{
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

    qDebug() << url << "→" << mime;

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
}

void GopherClient::on_finished()
{
    if(not was_cancelled)
    {
        emit this->requestComplete(this->body, mime);

        was_cancelled = true;
    }
    body.clear();
}
