#ifndef BROWSERTAB_HPP
#define BROWSERTAB_HPP

#include <QWidget>
#include <QUrl>
#include <QGraphicsScene>
#include <QTextDocument>

#include "geminiclient.hpp"
#include "documentoutlinemodel.hpp"
#include "tabbrowsinghistory.hpp"
#include "geminirenderer.hpp"

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
        PushAfterSuccess,
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

signals:
    void titleChanged(QString const & title);
    void locationChanged(QUrl const & url);

private slots:
    void on_url_bar_returnPressed();

    void on_refresh_button_clicked();

    void on_gemini_complete(QByteArray const & data, QString const & mime);

    void on_protocolViolation(QString const & reason);

    void on_inputRequired(QString const & query);

    void on_redirected(QUrl const & uri, bool is_permanent);

    void on_temporaryFailure(TemporaryFailure reason, QString const & info);

    void on_permanentFailure(PermanentFailure reason, QString const & info);

    void on_transientCertificateRequested(QString const & reason);

    void on_authorisedCertificateRequested(QString const & reason);

    void on_certificateRejected(CertificateRejection reason, QString const & info);

    void on_linkHovered(const QString &url);

    void on_fav_button_clicked();

    void on_text_browser_anchorClicked(const QUrl &arg1);

    void on_text_browser_highlighted(const QUrl &arg1);

    void on_back_button_clicked();

    void on_forward_button_clicked();

    void on_stop_button_clicked();

private:
    void setErrorMessage(QString const & msg);

    void pushToHistory(QUrl const & url);

    void updateUI();

public:

    Ui::BrowserTab *ui;
    MainWindow * mainWindow;
    QUrl current_location;

    GeminiClient gemini_client;
    int redirection_count = 0;

    bool push_to_history_after_load = false;
    bool successfully_loaded = false;

    DocumentOutlineModel outline;
    QGraphicsScene graphics_scene;
    TabBrowsingHistory history;
    QModelIndex current_history_index;

    std::unique_ptr<QTextDocument> current_document;
};

#endif // BROWSERTAB_HPP
