#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "browsertab.hpp"

#include <memory>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      url_status(new QLabel())
{
    ui->setupUi(this);

    this->statusBar()->addWidget(this->url_status);

    this->favourites.load("./favourites.db");

    ui->favourites_view->setModel(&favourites);

    this->ui->history_window->setVisible(false);
    this->ui->clientcert_window->setVisible(false);
    this->ui->bookmarks_window->setVisible(true);
}

MainWindow::~MainWindow()
{
    this->favourites.save("./favourites.db");
    delete ui;
}

BrowserTab * MainWindow::addEmptyTab(bool focus_new)
{
    BrowserTab * tab = new BrowserTab(this);

    connect(tab, &BrowserTab::titleChanged, this, &MainWindow::on_tab_titleChanged);

    int index = this->ui->browser_tabs->addTab(tab, "Page");

    if(focus_new) {
        this->ui->browser_tabs->setCurrentIndex(index);
    }

    return tab;
}

BrowserTab * MainWindow::addNewTab(bool focus_new, QUrl const & url)
{
    auto tab = addEmptyTab(focus_new);
    tab->navigateTo(url);
    return tab;
}

void MainWindow::setUrlPreview(const QUrl &url)
{
    if(url.isValid()) {
        auto str = url.toString();
        if(str.length() > 300) {
            str = str.mid(0, 300) + "...";
        }
        this->url_status->setText(str);
    }
    else {
        this->url_status->setText("");
    }
}


void MainWindow::on_browser_tabs_currentChanged(int index)
{
    if(index >= 0) {
        BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->widget(index));

        if(tab != nullptr) {
            this->ui->outline_view->setModel(&tab->outline);
            this->ui->outline_view->expandAll();

            this->ui->history_view->setModel(&tab->history);
        } else {
            this->ui->outline_view->setModel(nullptr);
            this->ui->history_view->setModel(nullptr);
        }
    } else {
        this->ui->outline_view->setModel(nullptr);
        this->ui->history_view->setModel(nullptr);
    }
}

void MainWindow::on_favourites_view_doubleClicked(const QModelIndex &index)
{
    if(auto url = this->favourites.get(index); url.isValid()) {
        this->addNewTab(true, url);
    }
}

void MainWindow::on_browser_tabs_tabCloseRequested(int index)
{
    delete this->ui->browser_tabs->widget(index);
}

void MainWindow::on_history_view_doubleClicked(const QModelIndex &index)
{
    BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->currentWidget());
    if(tab != nullptr) {
        tab->navigateBack(index);
    }
}

void MainWindow::on_tab_titleChanged(const QString &title)
{
   auto * tab = qobject_cast<BrowserTab*>(sender());
   if(tab != nullptr) {
       int index = this->ui->browser_tabs->indexOf(tab);
       assert(index >= 0);
       this->ui->browser_tabs->setTabText(index, title);
   }
}

void MainWindow::on_tab_locationChanged(const QUrl &url)
{
    auto * tab = qobject_cast<BrowserTab*>(sender());
    if(tab != nullptr) {
        int index = this->ui->browser_tabs->indexOf(tab);
        assert(index >= 0);
        this->ui->browser_tabs->setTabToolTip(index, url.toString());
    }
}
