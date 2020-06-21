#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "browsertab.hpp"
#include "settingsdialog.hpp"
#include <cassert>
#include <QMessageBox>
#include <memory>
#include <QShortcut>
#include <QKeySequence>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include "ioutil.hpp"
#include "kristall.hpp"

#include "certificatemanagementdialog.hpp"

MainWindow::MainWindow(QApplication * app, QWidget *parent) :
    QMainWindow(parent),
    application(app),
    ui(new Ui::MainWindow),
    url_status(new ElideLabel(this)),
    file_size(new QLabel(this)),
    file_mime(new QLabel(this)),
    load_time(new QLabel(this))
{
    ui->setupUi(this);

    this->url_status->setElideMode(Qt::ElideMiddle);

    this->statusBar()->addWidget(this->url_status);
    this->statusBar()->addPermanentWidget(this->file_mime);
    this->statusBar()->addPermanentWidget(this->file_size);
    this->statusBar()->addPermanentWidget(this->load_time);

    this->protocols.load(global_settings);

    global_settings.beginGroup("Theme");
    this->current_style.load(global_settings);
    global_settings.endGroup();

    ui->favourites_view->setModel(&global_favourites);

    this->ui->outline_window->setVisible(false);
    this->ui->history_window->setVisible(false);
    this->ui->bookmarks_window->setVisible(false);

    for(QDockWidget * dock : findChildren<QDockWidget *>())
    {
        QAction * act = this->ui->menuView ->addAction(dock->windowTitle());
        act->setCheckable(true);
        act->setChecked(dock->isVisible());
        act->setData(QVariant::fromValue(dock));
        connect(act, QOverload<bool>::of(&QAction::triggered), dock, &QDockWidget::setVisible);
    }

    connect(this->ui->menuNavigation, &QMenu::aboutToShow, [this]() {
        BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->currentWidget());
        if(tab != nullptr) {
            ui->actionAdd_to_favourites->setChecked(global_favourites.contains(tab->current_location));
        }
    });

    connect(this->ui->menuView, &QMenu::aboutToShow, [this]() {
        for(QAction * act : this->ui->menuView->actions())
        {
            auto * dock = qvariant_cast<QDockWidget*>(act->data());
            act->setChecked(dock->isVisible());
        }
    });

    {
        QShortcut * sc = new QShortcut(QKeySequence("Ctrl+L"), this);
        connect(sc, &QShortcut::activated, this, &MainWindow::on_focus_inputbar);
    }

    {
        global_settings.beginGroup("Window State");
        if(global_settings.contains("geometry")) {
            restoreGeometry(global_settings.value("geometry").toByteArray());
        }
        if(global_settings.contains("state")) {
            restoreState(global_settings.value("state").toByteArray());
        }
        global_settings.endGroup();
    }

    this->ui->favourites_view->setContextMenuPolicy(Qt::CustomContextMenu);
    this->ui->history_view->setContextMenuPolicy(Qt::CustomContextMenu);

    reloadTheme();
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
    connect(tab, &BrowserTab::fileLoaded, this, &MainWindow::on_tab_fileLoaded);

    int index = this->ui->browser_tabs->addTab(tab, "Page");

    if(focus_new) {
        this->ui->browser_tabs->setCurrentIndex(index);
    }

    if(load_default) {
        tab->navigateTo(QUrl(global_options.start_page), BrowserTab::DontPush);
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
    global_favourites.save(global_settings);
    this->protocols.save(global_settings);

    global_settings.beginGroup("Client Identities");
    global_identities.save(global_settings);
    global_settings.endGroup();

    global_settings.beginGroup("Trusted Servers");
    global_gemini_trust.save(global_settings);
    global_settings.endGroup();

    global_settings.beginGroup("Trusted HTTPS Servers");
    global_https_trust.save(global_settings);
    global_settings.endGroup();

    global_settings.beginGroup("Theme");
    this->current_style.save(global_settings);
    global_settings.endGroup();

    {
        global_settings.beginGroup("Window State");

        global_settings.setValue("geometry", saveGeometry());
        global_settings.setValue("state", saveState());

        global_settings.endGroup();
    }

    global_options.save(global_settings);

    global_settings.sync();
}

void MainWindow::on_browser_tabs_currentChanged(int index)
{
    if(index >= 0) {
        BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->widget(index));

        if(tab != nullptr) {
            this->ui->outline_view->setModel(&tab->outline);
            this->ui->outline_view->expandAll();

            this->ui->history_view->setModel(&tab->history);

            this->setFileStatus(tab->current_stats);
        } else {
            this->ui->outline_view->setModel(nullptr);
            this->ui->history_view->setModel(nullptr);
            this->setFileStatus(DocumentStats { });
        }
    } else {
        this->ui->outline_view->setModel(nullptr);
        this->ui->history_view->setModel(nullptr);
        this->setFileStatus(DocumentStats { });
    }
}

void MainWindow::on_favourites_view_doubleClicked(const QModelIndex &index)
{
    if(auto url = global_favourites.get(index); url.isValid()) {
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
    dialog.setProtocols(this->protocols);
    dialog.setOptions(global_options);
    dialog.setGeminiSslTrust(global_gemini_trust);
    dialog.setHttpsSslTrust(global_https_trust);

    if(dialog.exec() != QDialog::Accepted)
        return;

    global_gemini_trust = dialog.geminiSslTrust();
    global_https_trust = dialog.httpsSslTrust();
    global_options = dialog.options();

    this->protocols = dialog.protocols();
    this->current_style = dialog.geminiStyle();

    this->saveSettings();

    this->reloadTheme();
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

void MainWindow::reloadTheme()
{
    if(global_options.theme == Theme::os_default)
    {
        application->setStyleSheet("");
        QIcon::setThemeName("light");
    }
    if(global_options.theme == Theme::light)
    {
        QFile file(":/light.qss");
        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream stream(&file);
        application->setStyleSheet(stream.readAll());

        QIcon::setThemeName("light");
    }
    else if(global_options.theme == Theme::dark)
    {
        QFile file(":/dark.qss");
        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream stream(&file);
        application->setStyleSheet(stream.readAll());

        QIcon::setThemeName("dark");
    }
}

void MainWindow::setFileStatus(const DocumentStats &stats)
{
    if(stats.isValid()) {
        this->file_size->setText(IoUtil::size_human(stats.file_size));
        this->file_mime->setText(stats.mime_type.toString());
        this->load_time->setText(QString("%1 ms").arg(stats.loading_time));
    } else {
        this->file_size->setText("");
        this->file_mime->setText("");
        this->load_time->setText("");
    }
}

void MainWindow::on_actionSave_as_triggered()
{
    BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->currentWidget());
    if(tab != nullptr) {
        QFileDialog dialog { this };
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        dialog.selectFile(tab->current_location.fileName());

        if(dialog.exec() !=QFileDialog::Accepted)
            return;

        QString fileName = dialog.selectedFiles().at(0);

        QFile file { fileName };

        if(file.open(QFile::WriteOnly))
        {
            IoUtil::writeAll(file, tab->current_buffer);
        }
        else
        {
            QMessageBox::warning(this, "Kristall", QString("Could not save file:\r\n%1").arg(file.errorString()));
        }
    }
}

void MainWindow::on_actionGo_to_home_triggered()
{
    BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->currentWidget());
    if(tab != nullptr) {
        tab->navigateTo(QUrl(global_options.start_page), BrowserTab::PushImmediate);
    }
}

void MainWindow::on_actionAdd_to_favourites_triggered()
{
    BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->currentWidget());
    if(tab != nullptr) {
        tab->toggleIsFavourite();
    }
}

void MainWindow::on_tab_fileLoaded(DocumentStats const & stats)
{
    auto * tab = qobject_cast<BrowserTab*>(sender());
    if(tab != nullptr) {
        int index = this->ui->browser_tabs->indexOf(tab);
        assert(index >= 0);
        if(index == this->ui->browser_tabs->currentIndex()) {
            setFileStatus(stats);
            this->ui->outline_view->expandAll();
        }
    }
}

void MainWindow::on_focus_inputbar()
{
    BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->currentWidget());
    if(tab != nullptr) {
        tab->focusUrlBar();
    }
}

void MainWindow::on_actionHelp_triggered()
{
    this->addNewTab(true, QUrl("about:help"));
}

void MainWindow::on_history_view_customContextMenuRequested(const QPoint &pos)
{
    if(auto idx = this->ui->history_view->indexAt(pos); idx.isValid()) {
        BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->currentWidget());
        if(tab != nullptr) {
            if(QUrl url = tab->history.get(idx); url.isValid()) {
                QMenu menu;

                connect(menu.addAction("Open here"), &QAction::triggered, [tab, idx]() {
                    // We do the same thing as a double click here
                    tab->navigateBack(idx);
                });

                connect(menu.addAction("Open in new tab"), &QAction::triggered, [this, url]() {
                    addNewTab(true, url);
                });

                menu.exec(this->ui->history_view->mapToGlobal(pos));
            }
        }
    }
}

void MainWindow::on_favourites_view_customContextMenuRequested(const QPoint &pos)
{
    if(auto idx = this->ui->favourites_view->indexAt(pos); idx.isValid()) {
        if(QUrl url = global_favourites.get(idx); url.isValid()) {
            QMenu menu;

            BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->currentWidget());
            if(tab != nullptr) {
                connect(menu.addAction("Open here"), &QAction::triggered, [tab, url]() {
                    tab->navigateTo(url, BrowserTab::PushImmediate);
                });
            }

            connect(menu.addAction("Open in new tab"), &QAction::triggered, [this, url]() {
                addNewTab(true, url);
            });

            menu.exec(this->ui->favourites_view->mapToGlobal(pos));
        }
    }
}

void MainWindow::on_actionChangelog_triggered()
{
    this->addNewTab(true, QUrl("about:updates"));
}

void MainWindow::on_actionManage_Certificates_triggered()
{
    CertificateManagementDialog dialog { this };

    dialog.exec();

    this->saveSettings();
}
