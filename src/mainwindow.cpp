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
#include <QMouseEvent>

#include "ioutil.hpp"
#include "kristall.hpp"
#include "widgets/browsertabbar.hpp"

#include "dialogs/certificatemanagementdialog.hpp"

MainWindow::MainWindow(QApplication * app, QWidget *parent) :
    QMainWindow(parent),
    application(app),
    ui(new Ui::MainWindow),
    url_status(new ElideLabel(this)),
    file_size(new QLabel(this)),
    file_cached(new QLabel(this)),
    file_mime(new QLabel(this)),
    load_time(new QLabel(this))
{
    this->setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    this->url_status->setElideMode(Qt::ElideMiddle);

    this->statusBar()->addWidget(this->url_status);
    this->statusBar()->addPermanentWidget(this->file_cached);
    this->statusBar()->addPermanentWidget(this->file_mime);
    this->statusBar()->addPermanentWidget(this->file_size);
    this->statusBar()->addPermanentWidget(this->load_time);

    ui->favourites_view->setModel(&kristall::globals().favourites);

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
        BrowserTab * tab = this->curTab();
        if(tab != nullptr) {
            ui->actionAdd_to_favourites->setChecked(kristall::globals().favourites.containsUrl(tab->current_location));
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
            connect(sc, &QShortcut::activated, this, [this, tab]()
            {
                // 1-9 goes from the first to the n-th tab, 0 goes to the last one
                setCurrentTabIndex((tab == '0'
                    ? this->ui->browser_tabs->count()
                    : tab-'0') - 1);
            });
        }
    }

    {
        QShortcut * sc = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageDown), this);
        connect(sc, &QShortcut::activated, this, [this](){
           int i = this->currentTabIndex();

           if (i + 1 >= this->ui->browser_tabs->count())
               i = 0;
           else
               i++;

           this->setCurrentTabIndex(i);
        });
    }

    {
        QShortcut * sc = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageUp), this);
        connect(sc, &QShortcut::activated, this, [this](){
           int i = this->currentTabIndex();

           if (!i)
               i = this->ui->browser_tabs->count() - 1;
           else
               i--;

           this->setCurrentTabIndex(i);
        });
    }

    this->ui->favourites_view->setContextMenuPolicy(Qt::CustomContextMenu);
    this->ui->history_view->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this->ui->browser_tabs->tab_bar, &BrowserTabBar::on_newTabClicked, this, [this]() {
        this->addEmptyTab(true, true);
    });

    kristall::registerAppWindow(this);
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
    connect(tab, &BrowserTab::requestStateChanged, this, &MainWindow::on_tab_requestStateChanged);

    int index = this->ui->browser_tabs->addTab(tab, "Page");

    if(focus_new) {
        this->setCurrentTabIndex(index);
    }

    if(load_default) {
        tab->navigateTo(QUrl(kristall::globals().options.start_page), BrowserTab::PushImmediate);
        tab->focusUrlBar();
    } else {
        tab->navigateTo(QUrl("about:blank"), BrowserTab::DontPush);
    }

    return tab;
}

BrowserTab * MainWindow::addNewTab(bool focus_new, QUrl const & url)
{
    auto tab = addEmptyTab(focus_new, false);
    tab->navigateTo(url, BrowserTab::PushImmediate);
    return tab;
}

BrowserTab * MainWindow::curTab() const
{
    // Was getting irritated writing this out all the time
    return qobject_cast<BrowserTab*>(this->ui->browser_tabs->currentWidget());
}

BrowserTab * MainWindow::tabAt(int index) const
{
    return qobject_cast<BrowserTab*>(this->ui->browser_tabs->widget(index));
}

int MainWindow::tabCount() const
{
    return this->ui->browser_tabs->count();
}

void MainWindow::setUrlPreview(const QUrl &url)
{
    if(url.isValid()) {
        auto str = url.toString();
        if(str.length() > 300) {
            str = str.mid(0, 300) + "...";
        }
        this->previewing_url = true;
        this->url_status->setText(str);
        return;
    }

    this->previewing_url = false;
    this->url_status->setText(this->request_status);
}

void MainWindow::setRequestState(RequestState state)
{
    switch (state)
    {
    case RequestState::Started:
    {
        this->request_status = "Looking up...";
    } break;

    case RequestState::StartedWeb:
    {
        this->request_status = "Loading webpage...";
    } break;

    case RequestState::HostFound:
    {
        this->request_status = "Connecting...";
    } break;

    case RequestState::Connected:
    {
        this->request_status = "Downloading...";
    } break;

    default:
    {
        this->request_status = "";
    } break;
    }

    if (!this->previewing_url)
        this->url_status->setText(this->request_status);
}

void MainWindow::viewPageSource()
{
    BrowserTab * tab = this->curTab();
    if(tab != nullptr) {
        tab->openSourceView();
    }
}

void MainWindow::updateWindowTitle()
{
    BrowserTab * tab = this->curTab();
    if (tab == nullptr || tab->page_title.isEmpty())
    {
        this->setWindowTitle("Kristall");
        return;
    }
    this->setWindowTitle(QString("%0 - %1").arg(tab->page_title, "Kristall"));
}

void MainWindow::setUiDensity(UIDensity density, bool previewing)
{
    // If we are previewing, we only update the current tab.
    // If not, we update all tabs as it means user accepted the settings
    // dialog.

    if (previewing)
    {
        if (not this->curTab())
            return;
        this->curTab()->setUiDensity(density);
    }
    else
    {
        for (int i = 0; i < this->ui->browser_tabs->count(); ++i)
            this->tabAt(i)->setUiDensity(density);
    }
}

QString MainWindow::newGroupDialog()
{
    QInputDialog dialog { this };

    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setLabelText(tr("Enter name of the new group:"));

    if(dialog.exec() != QDialog::Accepted)
        return QString { };

    kristall::globals().favourites.addGroup(dialog.textValue());

    return dialog.textValue();
}

void MainWindow::applySettings()
{
    // Flag open tabs for re-render so theme
    // changes are instantly applied.
    for (int i = 0; i < this->ui->browser_tabs->count(); ++i)
    {
        BrowserTab *t = this->tabAt(i);
        t->refreshOptionalToolbarItems();
        t->refreshToolbarIcons();
        t->needs_rerender = true;
    }

    // Re-render the currently-open tab if we have one.
    BrowserTab * tab = this->curTab();
    if (tab)
        tab->rerenderPage();

    // Update new-tab button visibility.
    this->ui->browser_tabs->tab_bar->new_tab_btn->setVisible(kristall::globals().options.enable_newtab_btn);
}

int MainWindow::currentTabIndex()
{
    return this->ui->browser_tabs->currentIndex();
}

void MainWindow::setCurrentTabIndex(int index)
{
    this->ui->browser_tabs->setCurrentIndex(index);
}


void MainWindow::mousePressEvent(QMouseEvent *event)
{
    QMainWindow::mousePressEvent(event);

    BrowserTab * tab = this->curTab();
    if (tab == nullptr) return;

    // Navigate back/forward on mouse buttons 4/5
    if (event->buttons() == Qt::ForwardButton &&
        tab->history.oneForward(tab->current_history_index).isValid())
    {
        tab->navOneForward();
    }
    else if (event->buttons() == Qt::BackButton &&
        tab->history.oneBackward(tab->current_history_index).isValid())
    {
        tab->navOneBackward();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(kristall::getWindowCount() == 1) {
        kristall::saveSession();
    }
    event->accept();
}

void MainWindow::on_browser_tabs_currentChanged(int index)
{
    if(index >= 0) {
        BrowserTab * tab = this->tabAt(index);

        if(tab != nullptr) {
            this->ui->outline_view->setModel(&tab->outline);
            this->ui->outline_view->expandAll();

            this->ui->history_view->setModel(&tab->history);

            this->setFileStatus(tab->current_stats);

            if (tab->needs_rerender)
            {
                tab->rerenderPage();
            }
            else
            {
                tab->refreshFavButton();
            }

            this->setRequestState(tab->request_state);
        } else {
            this->ui->outline_view->setModel(nullptr);
            this->ui->history_view->setModel(nullptr);
            this->setFileStatus(DocumentStats { });
            this->setRequestState(RequestState::None);
        }
    } else {
        this->ui->outline_view->setModel(nullptr);
        this->ui->history_view->setModel(nullptr);
        this->setFileStatus(DocumentStats { });
        this->setRequestState(RequestState::None);
    }
    updateWindowTitle();
}

void MainWindow::on_browser_tabs_tabCloseRequested(int index)
{
    delete tabAt(index);
}

void MainWindow::on_tab_titleChanged(const QString &title)
{
    auto * tab = qobject_cast<BrowserTab*>(sender());
    if(tab != nullptr) {
        int index = this->ui->browser_tabs->indexOf(tab);
        assert(index >= 0);

        QString escapedTitle = title;

        // Set the window title to full title
        if (tab == this->curTab())
        {
            updateWindowTitle();
        }

        // Set tooltip
        this->ui->browser_tabs->tab_bar->setTabToolTip(index, title);

        // Shorten lengthy titles for tab bar (45 chars max for now - we assume
        // that Gemini surfers don't usually have loads of tabs open, so the
        // limit is fairly high)
        const int MAX_TITLE_LEN = 45;
        if (escapedTitle.length() > MAX_TITLE_LEN)
        {
            escapedTitle = escapedTitle.mid(0, MAX_TITLE_LEN - 3).trimmed() + "...";
        }

        this->ui->browser_tabs->setTabText(index, escapedTitle.replace("&", "&&"));
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
    BrowserTab * tab = this->curTab();
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

    dialog.setGeminiStyle(kristall::globals().document_style);
    dialog.setProtocols(kristall::globals().protocols);
    dialog.setOptions(kristall::globals().options);
    dialog.setGeminiSslTrust(kristall::globals().trust.gemini);
    dialog.setHttpsSslTrust(kristall::globals().trust.https);

    if(dialog.exec() != QDialog::Accepted) {
        kristall::setTheme(kristall::globals().options.theme);
        this->setUiDensity(kristall::globals().options.ui_density, false);
        return;
    }

    kristall::globals().trust.gemini = dialog.geminiSslTrust();
    kristall::globals().trust.https = dialog.httpsSslTrust();
    kristall::globals().options = dialog.options();

    kristall::globals().protocols = dialog.protocols();
    kristall::globals().document_style = dialog.geminiStyle();

    kristall::applySettings();

    kristall::saveSettings();
}

void MainWindow::on_actionNew_Tab_triggered()
{
    this->addEmptyTab(true, true);
}

void MainWindow::on_actionQuit_triggered()
{
    kristall::saveSession();
    QApplication::quit();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,
                       tr("Kristall"),
tr(R"about(Kristall, an OpenSource Gemini browser.
Made by Felix "xq" Queißner

This is free software. You can get the source code at
https://github.com/MasterQ32/Kristall)about")
    );
}

void MainWindow::on_actionClose_Tab_triggered()
{
    BrowserTab * tab = this->curTab();
    if(tab != nullptr) {
        delete tab;
    }
}

void MainWindow::on_actionForward_triggered()
{
    BrowserTab * tab = this->curTab();
    if(tab != nullptr) {
        tab->navOneForward();
    }
}

void MainWindow::on_actionBackward_triggered()
{
    BrowserTab * tab = this->curTab();
    if(tab != nullptr) {
        tab->navOneBackward();
    }
}

void MainWindow::on_actionRoot_triggered()
{
    BrowserTab * tab = this->curTab();
    if(tab != nullptr) {
        tab->navigateToRoot();
    }
}

void MainWindow::on_actionParent_triggered()
{
    BrowserTab * tab = this->curTab();
    if(tab != nullptr) {
        tab->navigateToParent();
    }
}

void MainWindow::on_actionRefresh_triggered()
{
    BrowserTab * tab = this->curTab();
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
        this->file_cached->setText(stats.loaded_from_cache ? tr("(cached)") : "");
        this->file_mime->setText(stats.mime_type.toString(false));
        this->load_time->setText(QString(tr("%1 ms")).arg(stats.loading_time));
    } else {
        this->file_size->setText("");
        this->file_cached->setText("");
        this->file_mime->setText("");
        this->load_time->setText("");
    }
}

void MainWindow::on_actionSave_as_triggered()
{
    BrowserTab * tab = this->curTab();
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
            QMessageBox::warning(this, tr("Kristall"), QString("Could not save file:\r\n%1").arg(file.errorString()));
        }
    }
}

void MainWindow::on_actionGo_to_home_triggered()
{
    BrowserTab * tab = this->curTab();
    if(tab != nullptr) {
        tab->navigateTo(QUrl(kristall::globals().options.start_page), BrowserTab::PushImmediate);
    }
}

void MainWindow::on_actionAdd_to_favourites_triggered()
{
    BrowserTab * tab = this->curTab();
    if(tab != nullptr) {
        tab->showFavouritesPopup();
    }
}

void MainWindow::on_tab_fileLoaded(DocumentStats const & stats)
{
    auto * tab = qobject_cast<BrowserTab*>(sender());
    if(tab != nullptr) {
        int index = this->ui->browser_tabs->indexOf(tab);
        assert(index >= 0);
        if(index == this->currentTabIndex()) {
            setFileStatus(stats);
            this->ui->outline_view->expandAll();
        }
    }
}

void MainWindow::on_tab_requestStateChanged(RequestState state)
{
    auto * tab = qobject_cast<BrowserTab*>(sender());
    if(tab != nullptr) {
        int index = this->ui->browser_tabs->indexOf(tab);
        assert(index >= 0);
        if(index == this->currentTabIndex()) {
            setRequestState(state);
        }
    }
}

void MainWindow::on_focus_inputbar()
{
    BrowserTab * tab = this->curTab();
    if(tab != nullptr) {
        tab->focusUrlBar();
    }
}

void MainWindow::on_actionHelp_triggered()
{
    this->addNewTab(true, QUrl("about:help"));
}

void MainWindow::on_history_view_customContextMenuRequested(const QPoint pos)
{
    if(auto idx = this->ui->history_view->indexAt(pos); idx.isValid()) {
        BrowserTab * tab = this->curTab();
        if(tab != nullptr) {
            if(QUrl url = tab->history.get(idx); url.isValid()) {
                QMenu menu;

                connect(menu.addAction(tr("Open here")), &QAction::triggered, [tab, idx]() {
                    // We do the same thing as a double click here
                    tab->navigateBack(idx);
                });

                connect(menu.addAction(tr("Open in new tab")), &QAction::triggered, [this, url]() {
                    addNewTab(true, url);
                });

                menu.exec(this->ui->history_view->mapToGlobal(pos));
            }
        }
    }
}

void MainWindow::on_favourites_view_customContextMenuRequested(const QPoint pos)
{
    if(auto idx = this->ui->favourites_view->indexAt(pos); idx.isValid()) {
        if(QUrl url = kristall::globals().favourites.getFavourite(idx).destination; url.isValid()) {
            QMenu menu;

            BrowserTab * tab = this->curTab();
            if(tab != nullptr) {
                connect(menu.addAction(tr("Open here")), &QAction::triggered, [tab, url]() {
                    tab->navigateTo(url, BrowserTab::PushImmediate);
                });
            }

            connect(menu.addAction(tr("Open in new tab")), &QAction::triggered, [this, url]() {
                addNewTab(true, url);
            });

            menu.addSeparator();

            connect(menu.addAction(tr("Relocate")), &QAction::triggered, [this, idx]() {
                QInputDialog dialog { this };

                dialog.setInputMode(QInputDialog::TextInput);
                dialog.setLabelText(tr("Enter new location of this favourite:"));
                dialog.setTextValue(kristall::globals().favourites.getFavourite(idx).destination.toString(QUrl::FullyEncoded));

                if (dialog.exec() != QDialog::Accepted)
                    return;

                kristall::globals().favourites.editFavouriteDest(idx, QUrl(dialog.textValue()));
            });

            connect(menu.addAction(tr("Rename")), &QAction::triggered, [this, idx]() {
                QInputDialog dialog { this };

                dialog.setInputMode(QInputDialog::TextInput);
                dialog.setLabelText(tr("New name of this favourite:"));
                dialog.setTextValue(kristall::globals().favourites.getFavourite(idx).getTitle());

                if (dialog.exec() != QDialog::Accepted)
                    return;

                kristall::globals().favourites.editFavouriteTitle(idx, dialog.textValue());
            });

            menu.addSeparator();

            connect(menu.addAction(tr("Delete")), &QAction::triggered, [idx]() {
                kristall::globals().favourites.destroyFavourite(idx);
            });

            menu.exec(this->ui->favourites_view->mapToGlobal(pos));
        }
        else if(QString group = kristall::globals().favourites.group(idx); not group.isEmpty()) {
            QMenu menu;

            connect(menu.addAction(tr("Rename group")), &QAction::triggered, [this, group]() {
                QInputDialog dialog { this };

                dialog.setInputMode(QInputDialog::TextInput);
                dialog.setLabelText(tr("New name of this group:"));
                dialog.setTextValue(group);

                if (dialog.exec() != QDialog::Accepted)
                    return;

                if (!kristall::globals().favourites.renameGroup(group, dialog.textValue()))
                    QMessageBox::information(this, tr("Kristall"), tr("Rename failed: group name already in use."));
            });

            menu.addSeparator();

            connect(menu.addAction(tr("Delete group")), &QAction::triggered, [this, idx]() {
                if (QMessageBox::question(
                    this,
                    tr("Kristall"),
                    tr("Are you sure you want to delete this Favourite Group?\n"
                    "All favourites in this group will be lost.\n\n"
                    "This action cannot be undone!")
                ) != QMessageBox::Yes)
                {
                    return;
                }
                kristall::globals().favourites.deleteGroupRecursive(kristall::globals().favourites.group(idx));
            });

            menu.exec(this->ui->favourites_view->mapToGlobal(pos));
        }
    }
    else {
        QMenu menu;

        connect(menu.addAction(tr("Create new group...")), &QAction::triggered, [this]() {
            this->newGroupDialog();
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

    dialog.setIdentitySet(kristall::globals().identities);
    if(dialog.exec() != QDialog::Accepted)
        return;

    kristall::globals().identities = dialog.identitySet();

    kristall::saveSettings();
}

void MainWindow::on_actionShow_document_source_triggered()
{
    this->viewPageSource();
}

void MainWindow::on_actionNew_window_triggered()
{
    kristall::openNewWindow(false);
}

void MainWindow::on_actionClose_Window_triggered()
{
    this->close();
}

void MainWindow::on_favourites_view_activated(const QModelIndex &index)
{
    if(auto url = kristall::globals().favourites.getFavourite(index).destination; url.isValid()) {
        this->addNewTab(true, url);
    }
}

void MainWindow::on_history_view_activated(const QModelIndex &index)
{
    BrowserTab * tab = this->curTab();
    if(tab != nullptr) {
        tab->navigateBack(index);
    }
}

void MainWindow::on_outline_view_activated(const QModelIndex &index)
{
    this->on_outline_view_clicked(index);
}
