#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


    void addEmptyTab();
    void addNewTab(QUrl const & url);

    void setUrlPreview(QUrl const & url);

private:
    Ui::MainWindow *ui;

    QLabel * url_status;
};
#endif // MAINWINDOW_HPP
