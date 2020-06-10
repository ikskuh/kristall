#include "certificateselectiondialog.hpp"
#include "ui_certificateselectiondialog.h"

#include "certificatehelper.hpp"

CertificateSelectionDialog::CertificateSelectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CertificateSelectionDialog)
{
    ui->setupUi(this);
    this->ui->server_request->setVisible(false);
}

CertificateSelectionDialog::~CertificateSelectionDialog()
{
    delete ui;
}

void CertificateSelectionDialog::setServerQuery(const QString &query)
{
    this->ui->server_request->setText(query);
    this->ui->server_request->setVisible(not query.isEmpty());
}

CryptoIdentity CertificateSelectionDialog::identity() const
{
    return cryto_identity;
}

void CertificateSelectionDialog::on_use_temp_cert_30m_clicked()
{
    acceptTemporaryWithTimeout(QDateTime::currentDateTime().addSecs(1800 * 12));
}

void CertificateSelectionDialog::on_use_temp_cert_1h_clicked()
{
    acceptTemporaryWithTimeout(QDateTime::currentDateTime().addSecs(3600));
}

void CertificateSelectionDialog::on_use_temp_cert_12h_clicked()
{
    acceptTemporaryWithTimeout(QDateTime::currentDateTime().addSecs(3600 * 12));
}

void CertificateSelectionDialog::on_use_temp_cert_24h_clicked()
{
    acceptTemporaryWithTimeout(QDateTime::currentDateTime().addDays(1));
}

void CertificateSelectionDialog::on_use_temp_cert_48h_clicked()
{
    acceptTemporaryWithTimeout(QDateTime::currentDateTime().addDays(2));
}

#include <QRandomGenerator>

void CertificateSelectionDialog::acceptTemporaryWithTimeout(QDateTime timeout)
{
    QRandomGenerator rng;

    char items[8];
    for(auto & c : items) {
        c = rng.bounded(std::numeric_limits<char>::min(), std::numeric_limits<char>::max());
    }

    this->cryto_identity = CertificateHelper::createNewIdentity(
        QByteArray(items, sizeof items).toBase64(QByteArray::OmitTrailingEquals),
        timeout);

    this->accept();
}
