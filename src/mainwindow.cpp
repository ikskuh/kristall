#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "browsertab.hpp"
#include "dialogs/settingsdialog.hpp"
#include <cassert>
#include <QMessageBox>
#include <memory>
#include <QShortcut>
#include <QKeySequence>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QInputDialog>

#include "ioutil.hpp"
#include "kristall.hpp"

#include "dialogs/certificatemanagementdialog.hpp"

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

    ui->favourites_view->setModel(&kristall::favourites);

    this->ui->outline_window->setVisible(false);
    this->ui->history_window->setVisible(false);
    this->ui->bookmarks_window->setVisible(false);

    for(QDockWidget * dock : findChildren<QDockWidget *>())
    {
        QAction * act = dock->toggleViewAction();
        act->setShortcut(dock->property("_shortcut").toString());
        // act->setIcon(dock->windowIcon());
        this->ui->menuView->addAction(act);
    }

    connect(this->ui->menuNavigation, &QMenu::aboutToShow, [this]() {
        BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->currentWidget());
        if(tab != nullptr) {
            ui->actionAdd_to_favourites->setChecked(kristall::favourites.containsUrl(tab->current_location));
        }
    });

    connect(this->ui->menuView, &QMenu::aboutToShow, [this]() {
        for(QAction * act : this->ui->menuView->actions())
        {
            auto * dock = qvariant_cast<QDockWidget*>(act->data());
            if(dock != nullptr) {
                act->setChecked(dock->isVisible());
            }
        }
    });

    {
        QShortcut * sc = new QShortcut(QKeySequence("Ctrl+L"), this);
        connect(sc, &QShortcut::activated, this, &MainWindow::on_focus_inputbar);
    }

    {
        std::string prefix = "Alt+";
        for (char tab = '0'; tab <= '9'; ++tab) {
	  std::string shortcut = prefix + tab;
          QShortcut * sc = new QShortcut(QKeySequence(shortcut.c_str()), this);
          connect(sc, &QShortcut::activated, this,
	  	[this, tab](){
		  // 1-9 goes from the first to the n-th tab, 0 goes to the last one
		  this->ui->browser_tabs->
		    setCurrentIndex((tab == '0' ?
				     this->ui->browser_tabs->count()
				    : tab-'0') - 1);
		});
      }
    }

    this->ui->favourites_view->setContextMenuPolicy(Qt::CustomContextMenu);
    this->ui->history_view->setContextMenuPolicy(Qt::CustomContextMenu);
}

MainWindow::~MainWindow()
{
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
        tab->navigateTo(QUrl(kristall::options.start_page), BrowserTab::PushImmediate);
    } else {
        tab->navigateTo(QUrl("about:blank"), BrowserTab::DontPush);
    }

    tab->focusUrlBar();

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

void MainWindow::viewPageSource()
{
    BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->currentWidget());
    if(tab != nullptr) {
        tab->openSourceView();
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

            this->setFileStatus(tab->current_stats);

            if (tab->needs_rerender)
            {
                tab->rerenderPage();
            }
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

//void MainWindow::on_favourites_view_doubleClicked(const QModelIndex &index)
//{
//    if(auto url = kristall::favourites.getFavourite(index).destination; url.isValid()) {
//        this->addNewTab(true, url);
//    }
//}

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

    dialog.setGeminiStyle(kristall::document_style);
    dialog.setProtocols(kristall::protocols);
    dialog.setOptions(kristall::options);
    dialog.setGeminiSslTrust(kristall::trust::gemini);
    dialog.setHttpsSslTrust(kristall::trust::https);

    if(dialog.exec() != QDialog::Accepted) {
        kristall::setTheme(kristall::options.theme);
        return;
    }

    kristall::trust::gemini = dialog.geminiSslTrust();
    kristall::trust::https = dialog.httpsSslTrust();
    kristall::options = dialog.options();

    kristall::protocols = dialog.protocols();
    kristall::document_style = dialog.geminiStyle();

    kristall::saveSettings();

    kristall::setTheme(kristall::options.theme);

    // Flag open tabs for re-render so theme
    // changes are instantly applied.
    for (int i = 0; i < this->ui->browser_tabs->count(); ++i)
    {
        qobject_cast<BrowserTab*>(this->ui->browser_tabs->widget(i))
            ->needs_rerender = true;
    }
    // Re-render the currently-open tab if we have one.
    BrowserTab * tab = qobject_cast<BrowserTab*>(this->ui->browser_tabs->currentWidget());
    if (tab) tab->rerenderPage();
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
        tab->navigateTo(QUrl(kristall::options.start_page), BrowserTab::PushImmediate);
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
        if(QUrl url = kristall::favourites.getFavourite(idx).destination; url.isValid()) {
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

            menu.addSeparator();

            connect(menu.addAction("Delete"), &QAction::triggered, [idx]() {
                kristall::favourites.destroyFavourite(idx);
            });

            menu.exec(this->ui->favourites_view->mapToGlobal(pos));
        }
        else if(QString group = kristall::favourites.group(idx); not group.isEmpty()) {
            qDebug() << group;
        }
    }
    else {
        QMenu menu;

        connect(menu.addAction("Create new group..."), &QAction::triggered, [this]() {
            QInputDialog dialog { this };

            dialog.setInputMode(QInputDialog::TextInput);
            dialog.setLabelText(tr("Enter name of the new group:"));

            if(dialog.exec() != QDialog::Accepted)
                return;

            kristall::favourites.addGroup(dialog.textValue());
        });

        menu.exec(this->ui->favourites_view->mapToGlobal(pos));
    }
}

void MainWindow::on_actionChangelog_triggered()
{
    this->addNewTab(true, QUrl("about:updates"));
}

void MainWindow::on_actionManage_Certificates_triggered()
{
    CertificateManagementDialog dialog { this };

    dialog.setIdentitySet(kristall::identities);
    if(dialog.exec() != QDialog::Accepted)
        return;

    kristall::identities = dialog.identitySet();

    kristall::saveSettings();
}

void MainWindow::on_actionShow_document_source_triggered()
{
    this->viewPageSource();
}
