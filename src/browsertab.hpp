#ifndef BROWSERTAB_HPP
#define BROWSERTAB_HPP

#include <memory>
#include <QWidget>
#include <QUrl>
#include <QGraphicsScene>
#include <QTextDocument>
#include <QNetworkAccessManager>
#include <QElapsedTimer>

#include "documentoutlinemodel.hpp"
#include "tabbrowsinghistory.hpp"
#include "geminirenderer.hpp"

#include "cryptoidentity.hpp"

#include "protocolhandler.hpp"

namespace Ui {
class BrowserTab;
}

class MainWindow;

class BrowserTab : public QWidget
{
    Q_OBJECT
public:
    enum PushToHistory {
        DontPush,
        PushImmediate,
    };

public:
    explicit BrowserTab(MainWindow * mainWindow);
    ~BrowserTab();

    void navigateTo(QUrl const & url, PushToHistory mode);

    void navigateBack(QModelIndex history_index);

    void navOneBackback();

    void navOneForward();

    void scrollToAnchor(QString const & anchor);

    void reloadPage();

    void toggleIsFavourite();

    void toggleIsFavourite(bool isFavourite);

    void focusUrlBar();

signals:
    void titleChanged(QString const & title);
    void locationChanged(QUrl const & url);
    void fileLoaded(qint64 fileSize, QString const & mime, int msec);

private slots:
    void on_url_bar_returnPressed();

    void on_refresh_button_clicked();

    void on_linkHovered(const QString &url);

    void on_fav_button_clicked();

    void on_text_browser_anchorClicked(const QUrl &arg1);

    void on_text_browser_highlighted(const QUrl &arg1);

    void on_back_button_clicked();

    void on_forward_button_clicked();

    void on_stop_button_clicked();

    void on_text_browser_customContextMenuRequested(const QPoint &pos);

    void on_enable_client_cert_button_clicked(bool checked);

private: // network slots

    void on_requestProgress(qint64 transferred);
    void on_requestComplete(QByteArray const & data, QString const & mime);
    void on_redirected(QUrl const & uri, bool is_permanent);
    void on_inputRequired(QString const & user_query);
    void on_networkError(ProtocolHandler::NetworkError error, QString const & reason);
    void on_certificateRequired(QString const & info);

private:
    void setErrorMessage(QString const & msg);

    void pushToHistory(QUrl const & url);

    void updateUI();

    bool trySetClientCertificate(QString const & query);

    void resetClientCertificate();

    void addProtocolHandler(std::unique_ptr<ProtocolHandler> && handler);

    template<typename T>
    void addProtocolHandler() {
        this->addProtocolHandler(std::make_unique<T>());
    }
public:

    Ui::BrowserTab *ui;
    MainWindow * mainWindow;
    QUrl current_location;

    std::vector<std::unique_ptr<ProtocolHandler>> protocol_handlers;

    ProtocolHandler * current_handler;

    int redirection_count = 0;

    bool successfully_loaded = false;

    DocumentOutlineModel outline;
    QGraphicsScene graphics_scene;
    TabBrowsingHistory history;
    QModelIndex current_history_index;

    std::unique_ptr<QTextDocument> current_document;

    QByteArray current_buffer;
    QString current_mime;
    QElapsedTimer timer;

    CryptoIdentity current_identitiy;
};

#endif // BROWSERTAB_HPP
