#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QLabel>
#include <QSettings>

#include "favouritecollection.hpp"
#include "renderers/geminirenderer.hpp"

#include "widgets/elidelabel.hpp"

#include "browsertab.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class BrowserTab;

enum class UIDensity : int;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QApplication * app, QWidget *parent = nullptr);
    ~MainWindow();

    BrowserTab * addEmptyTab(bool focus_new, bool load_default);
    BrowserTab * addNewTab(bool focus_new, QUrl const & url);
    BrowserTab * curTab() const;
    BrowserTab * tabAt(int index) const;

    void setUrlPreview(QUrl const & url);

    void viewPageSource();

    void updateWindowTitle();

    void setUiDensity(UIDensity density, bool previewing);

    QString newGroupDialog();

    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void on_browser_tabs_currentChanged(int index);

    void on_browser_tabs_tabCloseRequested(int index);

    void on_history_view_doubleClicked(const QModelIndex &index);

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

    void on_actionHelp_triggered();

    void on_history_view_customContextMenuRequested(const QPoint pos);

    void on_favourites_view_customContextMenuRequested(const QPoint pos);

    void on_favourites_view_doubleClicked(const QModelIndex &index);

    void on_actionChangelog_triggered();

    void on_actionManage_Certificates_triggered();

    void on_actionShow_document_source_triggered();

private: // slots

    void on_tab_fileLoaded(DocumentStats const & stats);

    void on_tab_titleChanged(QString const & title);

    void on_tab_locationChanged(QUrl const & url);

    void on_focus_inputbar();


private:
    void setFileStatus(DocumentStats const & stats);

public:
    QApplication * application;

private:
    Ui::MainWindow *ui;

    ElideLabel * url_status;
    QLabel * file_size;
    QLabel * file_cached;
    QLabel * file_mime;
    QLabel * load_time;
};
#endif // MAINWINDOW_HPP
