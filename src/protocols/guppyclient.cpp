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

    if(isInProgress())
        return false;

    if(url.scheme() != "guppy")
        return false;

    emit this->requestStateChange(RequestState::Started);

    this->requested_url = url;
    this->was_cancelled = false;
    this->prev_seq = this->first_seq = this->last_seq = 0;
    socket.connectToHost(url.host(), url.port(6775));

    return true;
}

bool GuppyClient::isInProgress() const
{
    return socket.isOpen();
}

bool GuppyClient::cancelRequest()
{
    was_cancelled = true;
    socket.close();
    timer.stop();
    body.clear();
    chunks.clear();
    return true;
}

void GuppyClient::on_connected()
{
    request = (requested_url.toString(QUrl::FormattingOptions(QUrl::FullyEncoded)) + "\r\n").toUtf8();
    if(socket.write(request.constData(), request.size()) <= 0)
    {
        socket.close();
        return;
    }

    timer.start(2000);

    emit this->requestStateChange(RequestState::Connected);
}

void GuppyClient::on_readRead()
{
    QByteArray chunk = socket.read(65535);

    if(int crlf = chunk.indexOf("\r\n"); crlf > 0) {
        QByteArray header = chunk.left(crlf);

        // first response packet (success, error or redirect) header should contain a space
        int seq = -1;
        if(int space = header.indexOf(' '); space > 0) {
            QByteArray meta = header.mid(space + 1);
            seq = chunk.left(space).toInt();

            if(seq < 6 || seq > 2147483647) {
                timer.stop();
                body.clear();
                chunks.clear();
                emit this->requestStateChange(RequestState::None);

                if(seq == 4) emit networkError(UnknownError, meta); // error
                else if (seq == 3) { // redirect
                    QUrl new_url(meta);

                    if(new_url.isRelative()) new_url = requested_url.resolved(new_url);
                    assert(not new_url.isRelative());

                    socket.close();
                    timer.stop();
                    body.clear();
                    chunks.clear();
                    emit this->requestStateChange(RequestState::None);

                    emit redirected(new_url, false);
                } else if (seq == 1) { // input prompt
                    socket.close();
                    timer.stop();
                    body.clear();
                    chunks.clear();

                    emit this->requestStateChange(RequestState::None);

                    emit inputRequired(meta, false);
                } else emit networkError(ProtocolViolation, QObject::tr("invalid seq"));

                return;
            }

            first_seq = seq; // success
            mime = meta;
        } else seq = header.toInt();

        if(seq < first_seq) return;
        if(chunk.size() == crlf + 2) last_seq = seq; // eof
        else if(seq >= first_seq) { // success or continuation
            if(!prev_seq || seq >= prev_seq) chunks[seq] = chunk.mid(crlf + 2, chunk.size() - crlf - 2);

            // postpone the timer when we receive the next packet
            if(seq == prev_seq + 1) timer.start();
        }
        // acknowledge every valid packet we receive
        QByteArray ack = QString("%1\r\n").arg(seq).toUtf8();
        socket.write(ack.constData(), ack.size());
    } else {
        emitNetworkError(socket.error(), socket.errorString());
        return;
    }

    // append all consequent chunks we have
    int next_seq = prev_seq ? prev_seq + 1 : first_seq;
    while(next_seq != last_seq) {
        QByteArray next = chunks.take(next_seq);
        if(next.isNull()) break;
        body.append(next.constData(), next.size());
        prev_seq = next_seq;
    }

    if(not was_cancelled) {
        emit this->requestProgress(body.size());
    }

    // we're done when the last appended chunk is the one before the eof chunk
    if(next_seq == last_seq) {
        if(not was_cancelled) {
            emit this->requestComplete(this->body, mime);
            was_cancelled = true;
        }
        socket.close();
        timer.stop();
        body.clear();
        chunks.clear();

        emit this->requestStateChange(RequestState::None);
    }
}

void GuppyClient::on_timerTick()
{
    QByteArray pkt;
    if(prev_seq) pkt = QString("%1\r\n").arg(prev_seq).toUtf8(); // resend the last ack
    else if(chunks.empty()) pkt = request; // resend the request
    else return;

    socket.write(pkt.constData(), pkt.size());
}
