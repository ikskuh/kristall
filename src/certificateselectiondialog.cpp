#include "certificateselectiondialog.hpp"
#include "ui_certificateselectiondialog.h"

#include "certificatehelper.hpp"
#include "kristall.hpp"
#include "newidentitiydialog.hpp"

#include <random>
#include <QDebug>
#include <QItemSelectionModel>

CertificateSelectionDialog::CertificateSelectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CertificateSelectionDialog)
{
    ui->setupUi(this);
    this->ui->server_request->setVisible(false);

    this->ui->certificates->setModel(&global_identities);
    this->ui->certificates->expandAll();

    connect(this->ui->certificates->selectionModel(), &QItemSelectionModel::currentChanged, this, &CertificateSelectionDialog::on_currentChanged);
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

void CertificateSelectionDialog::acceptTemporaryWithTimeout(QDateTime timeout)
{
    std::default_random_engine rng;
    rng.seed(QDateTime::currentDateTime().toMSecsSinceEpoch());

    std::uniform_int_distribution<int> distr(0, 255);

    char items[8];
    for(auto & c : items) {
        c = distr(rng);
    }

    this->cryto_identity = CertificateHelper::createNewIdentity(
        QByteArray(items, sizeof items).toBase64(QByteArray::OmitTrailingEquals),
        timeout);

    this->accept();
}

void CertificateSelectionDialog::on_currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    auto id = global_identities.getIdentity(current);

    this->ui->use_selected_cert->setEnabled(id.isValid());
}

void CertificateSelectionDialog::on_create_new_cert_clicked()
{
    NewIdentitiyDialog dialog { this };

    if(dialog.exec() != QDialog::Accepted)
        return;

    auto id = dialog.createIdentity();
    if(not id.isValid())
        return;
    id.is_persistent = true;

    global_identities.addCertificate(
        dialog.groupName(),
        id);
}

void CertificateSelectionDialog::on_use_selected_cert_clicked()
{
    auto sel = this->ui->certificates->selectionModel()->currentIndex();
    this->cryto_identity = global_identities.getIdentity(sel);
    if(this->cryto_identity.isValid()) {
        this->accept();
    } else {
        qDebug() << "Tried to use an invalid identity when the button should not be enabled. This is a bug!";
    }
}

void CertificateSelectionDialog::on_certificates_doubleClicked(const QModelIndex &index)
{
    this->cryto_identity = global_identities.getIdentity(index);
    if(this->cryto_identity.isValid()) {
        this->accept();
    } else {
        qDebug() << "Tried to use an invalid identity when the button should not be enabled. This is a bug!";
    }
}
