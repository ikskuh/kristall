#ifndef BROWSERTAB_HPP
#define BROWSERTAB_HPP

#include <memory>
#include <QWidget>
#include <QUrl>
#include <QGraphicsScene>
#include <QTextDocument>
#include <QNetworkAccessManager>
#include <QElapsedTimer>
#include <QTimer>
#include <QTextCursor>

#include "documentoutlinemodel.hpp"
#include "tabbrowsinghistory.hpp"
#include "renderers/geminirenderer.hpp"

#include "cryptoidentity.hpp"

#include "protocolhandler.hpp"

#include "mimeparser.hpp"

namespace Ui {
class BrowserTab;
}

class MainWindow;

enum class UIDensity : int;

struct DocumentStats
{
    int loading_time = 0; // in ms
    MimeType mime_type;
    qint64 file_size = 0;

    bool isValid() const {
        return mime_type.isValid();
    }
};

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

    void navigateBack(const QModelIndex &history_index);

    void navOneBackward();

    void navOneForward();

    void scrollToAnchor(QString const & anchor);

    void reloadPage();

    void toggleIsFavourite();

    void toggleIsFavourite(bool isFavourite);

    void focusUrlBar();

    void focusSearchBar();

    void openSourceView();

    void renderPage(const QByteArray & data, const MimeType & mime);

    void rerenderPage();

    void updatePageTitle();

    void setUrlBarText(const QString & text);

    void updateUrlBarStyle();

    void setUiDensity(UIDensity density);

signals:
    void titleChanged(QString const & title);
    void locationChanged(QUrl const & url);
    void fileLoaded(DocumentStats const & stats);

private slots:
    void on_url_bar_returnPressed();

    void on_url_bar_escapePressed();

    void on_url_bar_focused();

    void on_url_bar_blurred();

    void on_refresh_button_clicked();

    void on_fav_button_clicked();

    void on_text_browser_anchorClicked(const QUrl &arg1, bool open_in_new_tab);

    void on_text_browser_highlighted(const QUrl &arg1);

    void on_back_button_clicked();

    void on_forward_button_clicked();

    void on_stop_button_clicked();

    void on_home_button_clicked();

    void on_text_browser_customContextMenuRequested(const QPoint pos);

    void on_enable_client_cert_button_clicked(bool checked);

    void on_search_box_textChanged(const QString &arg1);

    void on_search_box_returnPressed();

    void on_search_next_clicked();

    void on_search_previous_clicked();

    void on_close_search_clicked();

private: // network slots

    void on_requestProgress(qint64 transferred);
    void on_requestComplete(QByteArray const & data, QString const & mime);
    void on_redirected(QUrl uri, bool is_permanent);
    void on_inputRequired(QString const & user_query, bool is_sensitive);
    void on_networkError(ProtocolHandler::NetworkError error, QString const & reason);
    void on_certificateRequired(QString const & info);
    void on_hostCertificateLoaded(QSslCertificate const & cert);

    void on_networkTimeout();

private: // ui slots
    void on_focusSearchbar();

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

    bool startRequest(QUrl const & url, ProtocolHandler::RequestOptions options);

    void updateMouseCursor(bool waiting);

    bool enableClientCertificate(CryptoIdentity const & ident);
    void disableClientCertificate();
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
    QSslCertificate current_server_certificate;

    QByteArray current_buffer;
    MimeType current_mime;
    QElapsedTimer timer;

    CryptoIdentity current_identity;

    bool is_internal_location;

    DocumentStats current_stats;

    QTimer network_timeout_timer;

    QTextCursor current_search_position;

    bool needs_rerender;

    QString page_title;

    bool no_url_style = false;
};

#endif // BROWSERTAB_HPP
