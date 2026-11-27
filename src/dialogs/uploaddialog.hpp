#ifndef UPLOADDIALOG_HPP
#define UPLOADDIALOG_HPP

#include <QDialog>

namespace Ui {
class UploadDialog;
}

class UploadDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UploadDialog(QWidget *parent = nullptr);
    ~UploadDialog();

    void setCurrentPage(QByteArray const & data, QString const & mime);

    QByteArray data() const;
    QString mimeType() const;
    QString token() const;

private slots:
    void on_text_mode_toggled(bool checked);

    void on_insert_page_button_clicked();

    void on_select_file_button_clicked();

    void on_file_name_textChanged(const QString &arg1);

    void on_text_input_textChanged();

private:
    void updateUI();

protected:
    void dragEnterEvent(QDragEnterEvent * event);
    void dropEvent(QDropEvent * event);

private:
    Ui::UploadDialog *ui;

    QByteArray page_data;
    QString page_mime;
};

#endif // UPLOADDIALOG_HPP
