#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QLabel>

#include "favouritecollection.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class BrowserTab;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


    BrowserTab * addEmptyTab(bool focus_new);
    BrowserTab * addNewTab(bool focus_new, QUrl const & url);

    void setUrlPreview(QUrl const & url);

public:
    FavouriteCollection favourites;

private slots:
    void on_browser_tabs_currentChanged(int index);

    void on_favourites_view_doubleClicked(const QModelIndex &index);

    void on_browser_tabs_tabCloseRequested(int index);

    void on_history_view_doubleClicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;

    QLabel * url_status;

};
#endif // MAINWINDOW_HPP
