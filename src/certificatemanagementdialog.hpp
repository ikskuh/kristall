#ifndef CERTIFICATEMANAGEMENTDIALOG_HPP
#define CERTIFICATEMANAGEMENTDIALOG_HPP

#include <QDialog>

#include "cryptoidentity.hpp"

namespace Ui {
class CertificateManagementDialog;
}

class CertificateManagementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CertificateManagementDialog(QWidget *parent = nullptr);
    ~CertificateManagementDialog();

private slots:
    void on_certificates_clicked(const QModelIndex &index);

    void on_cert_notes_textChanged();

    void on_cert_display_name_textChanged(const QString &arg1);

private:
    Ui::CertificateManagementDialog *ui;

    CryptoIdentity * selected_identity;
};

#endif // CERTIFICATEMANAGEMENTDIALOG_HPP
