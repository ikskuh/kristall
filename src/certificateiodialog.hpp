#ifndef CERTIFICATEIODIALOG_HPP
#define CERTIFICATEIODIALOG_HPP

#include <QDialog>
#include <QSsl>

namespace Ui {
class CertificateIoDialog;
}

class CertificateIoDialog : public QDialog
{
    Q_OBJECT
public:
    enum IoMode {
        Import,
        Export
    };
public:
    explicit CertificateIoDialog(QWidget *parent = nullptr);
    ~CertificateIoDialog();

    IoMode mode() const { return this->current_mode; }
    void setIoMode(IoMode mode);

    QSsl::KeyAlgorithm keyAlgorithm() const;
    void setKeyAlgorithm(QSsl::KeyAlgorithm alg);

    QString keyFileName() const;
    QString certificateFileName() const;

private slots:
    void on_select_certificate_file_button_clicked();

    void on_select_key_file_button_clicked();

    void on_certificate_file_name_textChanged(const QString &arg1);

    void on_key_file_name_textChanged(const QString &arg1);

private:
    void updateUI();

private:
    Ui::CertificateIoDialog *ui;

    IoMode current_mode;
};

#endif // CERTIFICATEIODIALOG_HPP
