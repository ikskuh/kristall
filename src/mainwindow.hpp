#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QLabel>
#include <QSettings>


#include "favouritecollection.hpp"
#include "geminirenderer.hpp"
#include "protocolsetup.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class BrowserTab;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QApplication * app, QWidget *parent = nullptr);
    ~MainWindow();

    BrowserTab * addEmptyTab(bool focus_new, bool load_default);
    BrowserTab * addNewTab(bool focus_new, QUrl const & url);

    void setUrlPreview(QUrl const & url);

    void saveSettings();

public:
    FavouriteCollection favourites;

private slots:
    void on_browser_tabs_currentChanged(int index);

    void on_favourites_view_doubleClicked(const QModelIndex &index);

    void on_browser_tabs_tabCloseRequested(int index);

    void on_history_view_doubleClicked(const QModelIndex &index);

    void on_tab_titleChanged(QString const & title);

    void on_tab_locationChanged(QUrl const & url);

    void on_outline_view_clicked(const QModelIndex &index);

    void on_actionSettings_triggered();

    void on_actionNew_Tab_triggered();

    void on_actionQuit_triggered();

    void on_actionAbout_triggered();

    void on_actionClose_Tab_triggered();

    void on_actionForward_triggered();

    void on_actionBackward_triggered();

    void on_actionRefresh_triggered();

    void on_actionAbout_Qt_triggered();

    void on_actionSave_as_triggered();

    void on_actionGo_to_home_triggered();

    void on_actionAdd_to_favourites_triggered();

    void on_tab_fileLoaded(qint64 fileSize, QString const & mime, int msec);

private:
    void reloadTheme();

public:
    QApplication * application;
    DocumentStyle current_style;
    ProtocolSetup protocols;

private:
    Ui::MainWindow *ui;

    QLabel * url_status;
    QLabel * file_size;
    QLabel * file_mime;
    QLabel * load_time;
};
#endif // MAINWINDOW_HPP
