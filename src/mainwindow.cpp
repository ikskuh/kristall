#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "browsertab.hpp"
#include "settingsdialog.hpp"

#include <QMessageBox>
#include <memory>
#include <QShortcut>
#include <QKeySequence>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    settings("xqTechnologies", "Kristall"),
    ui(new Ui::MainWindow),
    url_status(new QLabel())
{
    if(not this->settings.contains("start_page")) {
        this->settings.setValue("start_page", "about:favourites");
    }

    ui->setupUi(this);

    this->statusBar()->addWidget(this->url_status);

    this->favourites.load(settings);
    this->current_style.load(settings);
    this->protocols.load(settings);

    ui->favourites_view->setModel(&favourites);

    this->ui->outline_window->setVisible(false);
    this->ui->history_window->setVisible(false);
    this->ui->clientcert_window->setVisible(false);
    this->ui->bookmarks_window->setVisible(false);

    for(QDockWidget * dock : findChildren<QDockWidget *>())
    {
        QAction * act = this->ui->menuView ->addAction(dock->windowTitle());
        act->setCheckable(true);
        act->setChecked(dock->isVisible());
        act->setData(QVariant::fromValue(dock));
        connect(act, QOverload<bool>::of(&QAction::triggered), dock, &QDockWidget::setVisible);
    }

    connect(this->ui->menuView, &QMenu::aboutToShow, [this]() {
        for(QAction * act : this->ui->menuView->actions())
        {
            auto * dock = qvariant_cast<QDockWidget*>(act->data());
            act->setChecked(dock->isVisible());
        }
    });

    {
        settings.beginGroup("Window State");
        if(settings.contains("geometry")) {
            restoreGeometry(settings.value("geometry").toByteArray());
        }
        if(settings.contains("state")) {
            restoreState(settings.value("state").toByteArray());
        }
        settings.endGroup();
    }
}

MainWindow::~MainWindow()
{

    this->saveSettings();
    delete ui;
}

BrowserTab * MainWindow::addEmptyTab(bool focus_new, bool load_default)
{
    BrowserTab * tab = new BrowserTab(this);

    connect(tab, &BrowserTab::titleChanged, this, &MainWindow::on_tab_titleChanged);

    int index = this->ui->browser_tabs->addTab(tab, "Page");

    if(focus_new) {
        this->ui->browser_tabs->setCurrentIndex(index);
    }

    if(load_default) {
        tab->navigateTo(QUrl(this->settings.value("start_page").toString()), BrowserTab::DontPush);
    }

    return tab;
}

BrowserTab * MainWindow::addNewTab(bool focus_new, QUrl const & url)
{
    auto tab = addEmptyTab(focus_new, false);
    tab->navigateTo(url, BrowserTab::PushImmediate);
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

void MainWindow::saveSettings()
{
    this->favourites.save(settings);
    this->current_style.save(settings);
    this->protocols.save(settings);

    {
        settings.beginGroup("Window State");

        settings.setValue("geometry", saveGeometry());
        settings.setValue("state", saveState());

        settings.endGroup();
    }

    settings.sync();
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

void MainWindow::on_outline_view_clicked(const QModelIndex &index)
{
    BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->currentWidget());
    if(tab != nullptr) {

        auto anchor = tab->outline.getAnchor(index);
        if(not anchor.isEmpty()) {
            tab->scrollToAnchor(anchor);
        }
    }
}

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog dialog;

    dialog.setGeminiStyle(this->current_style);
    dialog.setStartPage(this->settings.value("start_page").toString());
    dialog.setProtocols(this->protocols);

    if(dialog.exec() != QDialog::Accepted)
        return;

    if(auto url = dialog.startPage(); url.isValid()) {
        this->settings.setValue("start_page", url.toString());
    }

    this->protocols = dialog.protocols();
    this->current_style = dialog.geminiStyle();
    this->saveSettings();
}

void MainWindow::on_actionNew_Tab_triggered()
{
    this->addEmptyTab(true, true);
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,
                       "Kristall",
R"about(Kristall, an OpenSource Gemini browser.
Made by Felix "xq" Queißner

This is free software. You can get the source code at
https://github.com/MasterQ32/Kristall)about"
    );
}

void MainWindow::on_actionClose_Tab_triggered()
{
    BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->currentWidget());
    if(tab != nullptr) {
        delete tab;
    }
}

void MainWindow::on_actionForward_triggered()
{
    BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->currentWidget());
    if(tab != nullptr) {
        tab->navOneForward();
    }
}

void MainWindow::on_actionBackward_triggered()
{
    BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->currentWidget());
    if(tab != nullptr) {
        tab->navOneBackback();
    }
}

void MainWindow::on_actionRefresh_triggered()
{
    BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->currentWidget());
    if(tab != nullptr) {
        tab->reloadPage();
    }
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, "Kristall");
}
