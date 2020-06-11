#include "newidentitiydialog.hpp"
#include "ui_newidentitiydialog.h"

#include "certificatehelper.hpp"
#include "kristall.hpp"

#include <QPushButton>
#include <QDebug>

NewIdentitiyDialog::NewIdentitiyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewIdentitiyDialog)
{
    ui->setupUi(this);

    ui->display_name->setText("Unnamed");
    ui->common_name->setText("Unnamed");
    ui->expiration_date->setDate(QDate::currentDate().addYears(1));
    ui->expiration_date->setTime(QTime(12, 00));

    ui->group->clear();
    for(auto group_name : global_identities.groups())
    {
        ui->group->addItem(group_name);
    }
}

NewIdentitiyDialog::~NewIdentitiyDialog()
{
    delete ui;
}

CryptoIdentity NewIdentitiyDialog::createIdentity() const
{
    auto id = CertificateHelper::createNewIdentity(
        this->ui->common_name->text(),
        this->ui->expiration_date->dateTime()
    );
    id.display_name = this->ui->display_name->text();
    return id;
}

QString NewIdentitiyDialog::groupName() const
{
    return this->ui->group->currentText();
}

void NewIdentitiyDialog::updateUI()
{
    bool is_ok = true;

    is_ok &= (not this->ui->group->currentText().isEmpty());
    is_ok &= (not this->ui->common_name->text().isEmpty());
    is_ok &= (not this->ui->display_name->text().isEmpty());
    is_ok &= (this->ui->expiration_date->dateTime() > QDateTime::currentDateTime());

    this->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(is_ok);
}

void NewIdentitiyDialog::on_group_editTextChanged(const QString &arg1)
{
    qDebug() << arg1;
    this->updateUI();
}

void NewIdentitiyDialog::on_display_name_textChanged(const QString &arg1)
{
    this->updateUI();
}

void NewIdentitiyDialog::on_common_name_textChanged(const QString &arg1)
{
    this->updateUI();
}
