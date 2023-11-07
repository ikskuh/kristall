#include "guppyclient.hpp"
#include "ioutil.hpp"
#include "kristall.hpp"

GuppyClient::GuppyClient(QObject *parent) : ProtocolHandler(parent)
{
    connect(&socket, &QUdpSocket::connected, this, &GuppyClient::on_connected);
    connect(&socket, &QUdpSocket::readyRead, this, &GuppyClient::on_readRead);
    connect(&timer, &QTimer::timeout, this, &GuppyClient::on_timerTick);

    connect(&socket, &QAbstractSocket::hostFound, this, [this]() {
        emit this->requestStateChange(RequestState::HostFound);
    });
    emit this->requestStateChange(RequestState::None);
}

GuppyClient::~GuppyClient()
{

}

bool GuppyClient::supportsScheme(const QString &scheme) const
{
    return (scheme == "guppy");
}

bool GuppyClient::startRequest(const QUrl &url, RequestOptions options)
{
    Q_UNUSED(options)

    if(this->isInProgress())
        return false;

    if(url.scheme() != "guppy")
        return false;

    emit this->requestStateChange(RequestState::Started);

    this->requested_url = url;
    this->was_cancelled = false;
    this->prev_seq = 0;
    this->first_seq = 0;
    this->last_seq = 0;
    this->socket.connectToHost(url.host(), url.port(6775));

    return true;
}

bool GuppyClient::isInProgress() const
{
    return this->socket.isOpen();
}

bool GuppyClient::cancelRequest()
{
    was_cancelled = true;
    this->socket.close();
    this->timer.stop();
    this->body.clear();
    this->chunks.clear();
    return true;
}

void GuppyClient::on_connected()
{
    request = (this->requested_url.toString(QUrl::FormattingOptions(QUrl::FullyEncoded)) + "\r\n").toUtf8();
    if(this->socket.write(request.constData(), request.size()) <= 0)
    {
        this->socket.close();
        return;
    }

    this->timer.start(2000);

    emit this->requestStateChange(RequestState::Connected);
}

void GuppyClient::on_readRead()
{
    QByteArray chunk = this->socket.read(65535);

    if(int crlf = chunk.indexOf("\r\n"); crlf > 0) {
        QByteArray header = chunk.left(crlf);

        // first response packet (success, error or redirect) header should contain a space
        int seq = -1;
        if(int space = header.indexOf(' '); space > 0) {
            QByteArray meta = header.mid(space + 1);
            seq = chunk.left(space).toInt();

            if(seq < 6 || seq > 2147483647) {
                this->timer.stop();
                this->body.clear();
                this->chunks.clear();
                emit this->requestStateChange(RequestState::None);

                if(seq == 4) {
                    emit networkError(UnknownError, meta); // error
                }
                else if(seq == 3) { // redirect
                    QUrl new_url(meta);

                    if(new_url.isRelative()) new_url = this->requested_url.resolved(new_url);
                    assert(not new_url.isRelative());

                    this->socket.close();
                    this->timer.stop();
                    this->body.clear();
                    this->chunks.clear();
                    emit this->requestStateChange(RequestState::None);

                    emit redirected(new_url, false);
                }
                else if(seq == 1) { // input prompt
                    this->socket.close();
                    this->timer.stop();
                    this->body.clear();
                    this->chunks.clear();

                    emit this->requestStateChange(RequestState::None);

                    emit inputRequired(meta, false);
                }
                else {
                    emit networkError(ProtocolViolation, QObject::tr("invalid seq"));
                }

                return;
            }

            this->first_seq = seq; // success
            this->mime = meta;
        }
        else {
            seq = header.toInt();
        }

        if(seq < this->first_seq) {
            return;
        }
        if(chunk.size() == crlf + 2) { // eof
            last_seq = seq;
        }
        else if(seq >= first_seq) { // success or continuation
            if(!this->prev_seq || seq >= this->prev_seq) {
                this->chunks[seq] = chunk.mid(crlf + 2, chunk.size() - crlf - 2);
            }

            // postpone the timer when we receive the next packet
            if(seq == this->prev_seq + 1) {
                this->timer.start();
            }
        }
        // acknowledge every valid packet we receive
        QByteArray ack = QString("%1\r\n").arg(seq).toUtf8();
        socket.write(ack.constData(), ack.size());
    }
    else {
        emitNetworkError(this->socket.error(), this->socket.errorString());
        return;
    }

    // append all consequent chunks we have
    int next_seq = this->prev_seq ? this->prev_seq + 1 : this->first_seq;
    while(next_seq != last_seq) {
        QByteArray next = this->chunks.take(next_seq);
        if(next.isNull()) {
            break;
        }
        body.append(next.constData(), next.size());
        this->prev_seq = next_seq++;
    }

    if(not this->was_cancelled) {
        emit this->requestProgress(body.size());
    }

    // we're done when the last appended chunk is the one before the eof chunk
    if(this->last_seq && next_seq == this->last_seq) {
        if(not this->was_cancelled) {
            emit this->requestComplete(this->body, this->mime);
            this->was_cancelled = true;
        }
        this->socket.close();
        this->timer.stop();
        this->body.clear();
        this->chunks.clear();

        emit this->requestStateChange(RequestState::None);
    }
}

void GuppyClient::on_timerTick()
{
    QByteArray pkt;
    if(this->prev_seq) { // resend the last ack
        pkt = QString("%1\r\n").arg(this->prev_seq).toUtf8();
    }
    else if(this->chunks.empty()) { // resend the request
        pkt = request;
    }
    else {
        return;
    }

    this->socket.write(pkt.constData(), pkt.size());
}
