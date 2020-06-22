#include "certificateiodialog.hpp"
#include "ui_certificateiodialog.h"

#include <QFileDialog>
#include <QPushButton>
#include <QSsl>

CertificateIoDialog::CertificateIoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CertificateIoDialog)
{
    ui->setupUi(this);

    this->ui->key_type->clear();
    this->ui->key_type->addItem("RSA", QVariant::fromValue<int>(QSsl::Rsa));
    this->ui->key_type->addItem("ECDSA", QVariant::fromValue<int>(QSsl::Ec));

    this->updateUI();
}

CertificateIoDialog::~CertificateIoDialog()
{
    delete ui;
}

void CertificateIoDialog::setIoMode(CertificateIoDialog::IoMode mode)
{
    this->current_mode = mode;
    if(mode == Export) {
        this->setWindowTitle(tr("Export Certificate"));
    } else {
        this->setWindowTitle(tr("Import Certificate"));
    }
    this->ui->key_type->setEnabled(mode == Import);
    this->updateUI();
}

QSsl::KeyAlgorithm CertificateIoDialog::keyAlgorithm() const
{
    return QSsl::KeyAlgorithm(this->ui->key_type->currentData().toInt());
}

void CertificateIoDialog::setKeyAlgorithm(QSsl::KeyAlgorithm alg)
{
    this->ui->key_type->setCurrentIndex(-1);
    for(int i = 0; i< this->ui->key_type->count(); i++) {
        if(this->ui->key_type->itemData(i).toInt() == int(alg)) {
            this->ui->key_type->setCurrentIndex(i);
            break;
        }
    }
}

QString CertificateIoDialog::keyFileName() const
{
    return this->ui->key_file_name->text();
}

QString CertificateIoDialog::certificateFileName() const
{
    return this->ui->certificate_file_name->text();
}

void CertificateIoDialog::on_select_certificate_file_button_clicked()
{
    QFileDialog dialog { this };

    dialog.setNameFilter("Certificate File(*.pem *.der)");
    dialog.setAcceptMode((this->current_mode == Export) ? QFileDialog::AcceptSave : QFileDialog::AcceptOpen);
    dialog.selectFile(this->ui->certificate_file_name->text());

    if(dialog.exec() != QDialog::Accepted)
        return;

    this->ui->certificate_file_name->setText(dialog.selectedFiles().first());

    this->updateUI();
}

void CertificateIoDialog::on_select_key_file_button_clicked()
{
    QFileDialog dialog { this };

    dialog.setNameFilter("Certificate File(*.pem *.der)");
    dialog.setAcceptMode((this->current_mode == Export) ? QFileDialog::AcceptSave : QFileDialog::AcceptOpen);
    dialog.selectFile(this->ui->key_file_name->text());

    if(dialog.exec() != QDialog::Accepted)
        return;

    this->ui->key_file_name->setText(dialog.selectedFiles().first());

    this->updateUI();
}

void CertificateIoDialog::on_certificate_file_name_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    this->updateUI();
}

void CertificateIoDialog::on_key_file_name_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    this->updateUI();
}

void CertificateIoDialog::updateUI()
{
    QString cert_file_name = certificateFileName();
    QString key_file_name = keyFileName();

    bool ok = true;

    ok &= (cert_file_name.endsWith(".pem") or cert_file_name.endsWith(".der"));
    ok &= (key_file_name.endsWith(".pem") or key_file_name.endsWith(".der"));

    ok &= (this->ui->key_type->currentIndex() >= 0);

    if(current_mode == Import) {
       ok &= QFile(cert_file_name).exists();
       ok &= QFile(key_file_name).exists();
    }

    this->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ok);
}
