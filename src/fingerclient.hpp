#ifndef FINGERCLIENT_HPP
#define FINGERCLIENT_HPP

#include <QObject>
#include <QTcpSocket>
#include <QUrl>

class FingerClient : public QObject
{
    Q_OBJECT
public:
    explicit FingerClient(QObject *parent = nullptr);

    ~FingerClient() override;

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
    bool was_cancelled;
    QString requested_user;
};

#endif // FINGERCLIENT_HPP
