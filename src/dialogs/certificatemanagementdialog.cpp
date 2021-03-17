#include "certificatemanagementdialog.hpp"
#include "ui_certificatemanagementdialog.h"

#include "kristall.hpp"

#include "newidentitiydialog.hpp"
#include "certificateiodialog.hpp"
#include "ioutil.hpp"

#include <QCryptographicHash>
#include <QMessageBox>

CertificateManagementDialog::CertificateManagementDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CertificateManagementDialog),
    selected_identity { nullptr }
{
    ui->setupUi(this);

    connect( // connect with "this" as context, so the connection will die when the window is destroyed
        kristall::globals().localization.get(), &Localization::translationChanged,
        this, [this]() { this->ui->retranslateUi(this); },
        Qt::DirectConnection
    );

    this->ui->certificates->setModel(&identity_set);
    this->ui->certificates->expandAll();

    connect(
        this->ui->certificates->selectionModel(),
        &QItemSelectionModel::currentChanged,
        this,
        &CertificateManagementDialog::on_certificates_selected
    );
    on_certificates_selected(QModelIndex { }, QModelIndex { });
}

CertificateManagementDialog::~CertificateManagementDialog()
{
    delete ui;
}

IdentityCollection CertificateManagementDialog::identitySet() const
{
    return this->identity_set;
}

void CertificateManagementDialog::setIdentitySet(const IdentityCollection &src)
{
    this->identity_set = src;
    this->ui->certificates->expandAll();

}

void CertificateManagementDialog::on_certificates_selected(QModelIndex const& index, QModelIndex const & previous)
{
    Q_UNUSED(previous);

    selected_identity = identity_set.getMutableIdentity(index);

    this->ui->export_cert_button->setEnabled(selected_identity != nullptr);

    if(selected_identity != nullptr)
    {
        auto & cert = *selected_identity;
        this->ui->groupBox->setEnabled(true);
        this->ui->cert_display_name->setText(cert.display_name);
        this->ui->cert_common_name->setText(cert.certificate.subjectInfo(QSslCertificate::CommonName).join(", "));
        this->ui->cert_expiration_date->setDateTime(cert.certificate.expiryDate());
        auto days = QDateTime::currentDateTime().daysTo(cert.certificate.expiryDate());
        this->ui->cert_livetime->setText(QString(tr("%1 day","%1 days",days)).arg(days));
        this->ui->cert_fingerprint->setPlainText(toFingerprintString(cert.certificate));
        this->ui->cert_notes->setPlainText(cert.user_notes);

        this->ui->cert_host_filter->setText(cert.host_filter);
        this->ui->cert_auto_enable->setEnabled(not cert.host_filter.isEmpty());
        this->ui->cert_auto_enable->setChecked(cert.auto_enable);

        this->ui->delete_cert_button->setEnabled(true);
    }
    else
    {
        this->ui->groupBox->setEnabled(false);
        this->ui->cert_display_name->setText("");
        this->ui->cert_common_name->setText("");
        this->ui->cert_expiration_date->setDateTime(QDateTime { });
        this->ui->cert_livetime->setText("");
        this->ui->cert_fingerprint->setPlainText("");
        this->ui->cert_host_filter->setText("");
        this->ui->cert_auto_enable->setChecked(false);

        if(auto group_name = identity_set.group(index); not group_name.isEmpty()) {
            this->ui->delete_cert_button->setEnabled(identity_set.canDeleteGroup(group_name));
        } else {
            this->ui->delete_cert_button->setEnabled(false);
        }
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
    Q_UNUSED(arg1)
    if(this->selected_identity != nullptr) {
        this->selected_identity->display_name = this->ui->cert_display_name->text();
    }
}

void CertificateManagementDialog::on_delete_cert_button_clicked()
{
    auto index = this->ui->certificates->currentIndex();

    if(identity_set.getMutableIdentity(index) != nullptr)
    {
        auto answer = QMessageBox::question(
            this,
            tr("Kristall"),
            tr("Do you really want to delete this certificate?\r\n\r\nYou will not be able to restore the identity after this!"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        if(answer != QMessageBox::Yes)
            return;
        if(not identity_set.destroyIdentity(index)) {
            QMessageBox::warning(this, tr("Kristall"), tr("Could not destroy identity!"));
        }
    }
    else if(auto group_name = identity_set.group(index); not group_name.isEmpty()) {

        auto answer = QMessageBox::question(
            this,
            tr("Kristall"),
            QString(tr("Do you want to delete the group '%1'")).arg(group_name)
        );
        if(answer != QMessageBox::Yes)
            return;

        if(not identity_set.deleteGroup(group_name)) {
            QMessageBox::warning(this, tr("Kristall"), tr("Could not delete group!"));
        }
    }
}

void CertificateManagementDialog::on_export_cert_button_clicked()
{
    if(this->selected_identity == nullptr)
        return;
    CertificateIoDialog dialog { this };

    dialog.setKeyAlgorithm(this->selected_identity->private_key.algorithm());
    dialog.setIoMode(CertificateIoDialog::Export);

    if(dialog.exec() != QDialog::Accepted)
        return;

    {
        QFile cert_file { dialog.certificateFileName() };
        if(not cert_file.open(QFile::WriteOnly)) {
            QMessageBox::warning(
                this,
                tr("Kristall"),
                tr("The file %1 could not be found!").arg(dialog.certificateFileName())
            );
            return;
        }

        QByteArray cert_blob;
        if(dialog.certificateFileName().endsWith(".der")) {
            cert_blob = this->selected_identity->certificate.toDer();
        } else {
            cert_blob = this->selected_identity->certificate.toPem();
        }

        if(not IoUtil::writeAll(cert_file, cert_blob)) {
            QMessageBox::warning(
                this,
                tr("Kristall"),
                tr("The file %1 could not be created found!").arg(dialog.certificateFileName())
            );
            return;
        }
    }

    {
        QFile key_file { dialog.keyFileName() };
        if(not key_file.open(QFile::WriteOnly)) {
            QMessageBox::warning(
                this,
                tr("Kristall"),
                tr("The file %1 could not be found!").arg(dialog.keyFileName())
            );
            return;
        }

        QByteArray key_blob;
        if(dialog.keyFileName().endsWith(".der")) {
            key_blob = this->selected_identity->private_key.toDer();
        } else {
            key_blob = this->selected_identity->private_key.toPem();
        }

        if(not IoUtil::writeAll(key_file, key_blob)) {
            QMessageBox::warning(
                this,
                tr("Kristall"),
                tr("The file %1 could not be created!").arg(dialog.keyFileName())
            );
            return;
        }
    }
}

void CertificateManagementDialog::on_import_cert_button_clicked()
{
    CertificateIoDialog dialog { this };

    dialog.setIoMode(CertificateIoDialog::Import);

    if(dialog.exec() != QDialog::Accepted)
        return;

    QFile cert_file { dialog.certificateFileName() };
    if(not cert_file.open(QFile::ReadOnly)) {
        QMessageBox::warning(
            this,
            tr("Kristall"),
            tr("The file %1 could not be found!").arg(dialog.certificateFileName())
        );
        return;
    }

    QFile key_file { dialog.keyFileName() };
    if(not key_file.open(QFile::ReadOnly)) {
        QMessageBox::warning(
            this,
            tr("Kristall"),
            tr("The file %1 could not be found!").arg(dialog.keyFileName())
        );
        return;
    }

    CryptoIdentity ident;
    ident.private_key = QSslKey {
        &key_file,
        dialog.keyAlgorithm(),
        dialog.keyFileName().endsWith(".der") ? QSsl::Der : QSsl::Pem,
        QSsl::PrivateKey
    };
    ident.certificate = QSslCertificate {
        &cert_file,
        dialog.keyFileName().endsWith(".der") ? QSsl::Der : QSsl::Pem,
    };
    ident.user_notes = tr("Imported from:\r\nkey: %1\r\n:cert: %2").arg(dialog.keyFileName(), dialog.certificateFileName());
    //: Default name
    ident.display_name = tr("Imported Certificate");
    ident.auto_enable = false;
    ident.host_filter = "";
    ident.is_persistent = true;

    if(ident.private_key.isNull()) {
        QMessageBox::warning(
            this,
            tr("Kristall"),
            tr("The key file %1 could not be loaded. Please verify your key file.").arg(dialog.keyFileName())
        );
        return;
    }

    if(ident.certificate.isNull()) {
        QMessageBox::warning(
            this,
            tr("Kristall"),
            tr("The certificate file %1 could not be loaded. Please verify your certificate.").arg(dialog.keyFileName())
        );
        return;
    }

    if(not identity_set.addCertificate(tr("Imported Certificates"), ident)) {
        QMessageBox::warning(
            this,
            tr("Kristall"),
            tr("Failed to import the certificate.")
        );
    }
}

void CertificateManagementDialog::on_create_cert_button_clicked()
{
    NewIdentitiyDialog dialog { this };

    dialog.setGroupName(identity_set.group(this->ui->certificates->currentIndex()));

    if(dialog.exec() != QDialog::Accepted)
        return;

    auto id = dialog.createIdentity();
    if(not id.isValid())
        return;
    id.is_persistent = true;

    identity_set.addCertificate(
        dialog.groupName(),
        id);
}

void CertificateManagementDialog::on_cert_host_filter_textChanged(const QString &host_filter)
{
    if(this->selected_identity != nullptr) {
        this->ui->cert_auto_enable->setEnabled(not host_filter.isEmpty());
        this->selected_identity->host_filter = host_filter;
    } else {
        this->ui->cert_auto_enable->setEnabled(false);
    }

}

void CertificateManagementDialog::on_cert_auto_enable_clicked(bool checked)
{
    if(this->selected_identity != nullptr) {
        this->selected_identity->auto_enable = checked;
    }
}
