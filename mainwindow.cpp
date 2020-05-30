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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addEmptyTab()
{
    auto tab = std::make_unique<BrowserTab>(this);



    this->ui->browser_tabs->addTab(tab.release(), "Page");
}

void MainWindow::addNewTab(QUrl const & url)
{
    auto tab = std::make_unique<BrowserTab>(this);

    tab->navigateTo(url);

    this->ui->browser_tabs->addTab(tab.release(), "Page");
}

void MainWindow::setUrlPreview(const QUrl &url)
{
    if(url.isValid())
        this->url_status->setText(url.toString());
    else
        this->url_status->setText("");
}

