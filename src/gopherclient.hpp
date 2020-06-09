#ifndef GOPHERCLIENT_HPP
#define GOPHERCLIENT_HPP

#include <QObject>
#include <QTcpSocket>
#include <QUrl>

class GopherClient : public QObject
{
    Q_OBJECT
public:
    explicit GopherClient(QObject *parent = nullptr);

    ~GopherClient() override;

    bool startRequest(QUrl const & url);

    bool isInProgress() const;

    bool cancelRequest();

signals:
    void requestProgress(qint64 transferred);

    void requestComplete(QByteArray const & data, QString const & mime);

    void requestFailed(QString const & message);

private slots:
    void on_connected();
    void on_readRead();
    void on_finished();

private:
    QTcpSocket socket;
    QByteArray body;
    QUrl requested_url;
    bool was_cancelled;
    QString mime;
    bool is_processing_binary;
};

#endif // GOPHERCLIENT_HPP
