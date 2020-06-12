#include "certificatemanagementdialog.hpp"
#include "ui_certificatemanagementdialog.h"

#include "kristall.hpp"

#include <QCryptographicHash>

CertificateManagementDialog::CertificateManagementDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CertificateManagementDialog),
    selected_identity { nullptr }
{
    ui->setupUi(this);

    this->ui->certificates->setModel(&global_identities);
    this->ui->certificates->expandAll();

    on_certificates_clicked(QModelIndex { });
}

CertificateManagementDialog::~CertificateManagementDialog()
{
    delete ui;
}

void CertificateManagementDialog::on_certificates_clicked(const QModelIndex &index)
{
    selected_identity = global_identities.getMutableIdentity(index);

    this->ui->delete_cert_button->setEnabled(selected_identity != nullptr);
    this->ui->export_cert_button->setEnabled(selected_identity != nullptr);

    if(selected_identity != nullptr)
    {
        auto & cert = *selected_identity;
        this->ui->groupBox->setEnabled(true);
        this->ui->cert_display_name->setText(cert.display_name);
        this->ui->cert_common_name->setText(cert.certificate.subjectInfo(QSslCertificate::CommonName).join(", "));
        this->ui->cert_expiration_date->setDateTime(cert.certificate.expiryDate());
        this->ui->cert_livetime->setText(QString("%1 days").arg(QDateTime::currentDateTime().daysTo(cert.certificate.expiryDate())));
        this->ui->cert_fingerprint->setPlainText(
            QCryptographicHash::hash(cert.certificate.toDer(), QCryptographicHash::Sha256).toHex(':')
        );
        this->ui->cert_notes->setPlainText(cert.user_notes);

    }
    else
    {
        this->ui->groupBox->setEnabled(false);
        this->ui->cert_display_name->setText("");
        this->ui->cert_common_name->setText("");
        this->ui->cert_expiration_date->setDateTime(QDateTime { });
        this->ui->cert_livetime->setText("");
        this->ui->cert_fingerprint->setPlainText("");
    }
}

void CertificateManagementDialog::on_cert_notes_textChanged()
{
    if(this->selected_identity != nullptr) {
        this->selected_identity->user_notes = this->ui->cert_notes->toPlainText();
    }
}

void CertificateManagementDialog::on_cert_display_name_textChanged(const QString &arg1)
{
    if(this->selected_identity != nullptr) {
        this->selected_identity->display_name = this->ui->cert_display_name->text();
    }
}
