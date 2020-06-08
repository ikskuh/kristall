#ifndef WEBCLIENT_HPP
#define WEBCLIENT_HPP

#include <QObject>
#include <QNetworkAccessManager>

class WebClient: public QObject
{
private:
    Q_OBJECT
public:
    explicit WebClient(QObject *parent = nullptr);

    ~WebClient() override;

    bool startRequest(QUrl const & url);

    bool isInProgress() const;

    bool cancelRequest();

signals:
    void requestProgress(qint64 transferred);

    void requestComplete(QByteArray const & data, QString const & mime);

    void requestFailed(QString const & message);

private slots:
    void on_data();
    void on_finished();

private:
    QNetworkAccessManager manager;
    QNetworkReply * current_reply;

    QByteArray body;
};

#endif // WEBCLIENT_HPP
