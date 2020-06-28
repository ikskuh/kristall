#ifndef CERTIFICATEMANAGEMENTDIALOG_HPP
#define CERTIFICATEMANAGEMENTDIALOG_HPP

#include <QDialog>

#include "cryptoidentity.hpp"
#include "identitycollection.hpp"

namespace Ui {
class CertificateManagementDialog;
}

class CertificateManagementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CertificateManagementDialog(QWidget *parent = nullptr);
    ~CertificateManagementDialog();

    IdentityCollection identitySet() const;
    void setIdentitySet(IdentityCollection const & src);

private slots:
    void on_cert_notes_textChanged();

    void on_cert_display_name_textChanged(const QString &arg1);

    void on_delete_cert_button_clicked();

    void on_export_cert_button_clicked();

    void on_import_cert_button_clicked();

    void on_create_cert_button_clicked();

    void on_cert_host_filter_textChanged(const QString &arg1);

    void on_cert_auto_enable_clicked(bool checked);

private:
    void on_certificates_selected(const QModelIndex &index, QModelIndex const & previous);
private:
    Ui::CertificateManagementDialog *ui;

    IdentityCollection identity_set;

    CryptoIdentity * selected_identity;
};

#endif // CERTIFICATEMANAGEMENTDIALOG_HPP
