#include "fingerclient.hpp"
#include "ioutil.hpp"

FingerClient::FingerClient(QObject *parent) : QObject(parent)
{
    connect(&socket, &QTcpSocket::connected, this, &FingerClient::on_connected);
    connect(&socket, &QTcpSocket::readyRead, this, &FingerClient::on_readRead);
    connect(&socket, &QTcpSocket::disconnected, this, &FingerClient::on_finished);
}

FingerClient::~FingerClient()
{

}

bool FingerClient::startRequest(const QUrl &url)
{
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
    socket.close();
    body.clear();
    return true;
}

void FingerClient::on_connected()
{
    auto blob = (requested_user + "\r\n").toUtf8();

    IoUtil::writeAll(socket, blob);
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
        emit this->requestComplete(this->body, "text/plain");
        was_cancelled = true;
    }
    body.clear();
}
