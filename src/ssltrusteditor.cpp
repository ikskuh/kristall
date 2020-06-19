#include "ssltrusteditor.hpp"
#include "ui_ssltrusteditor.h"

SslTrustEditor::SslTrustEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SslTrustEditor)
{
    ui->setupUi(this);

    this->ui->trust_level->clear();
    this->ui->trust_level->addItem("Trust on first encounter", QVariant::fromValue<int>(SslTrust::TrustOnFirstUse));
    this->ui->trust_level->addItem("Trust everything", QVariant::fromValue<int>(SslTrust::TrustEverything));
    this->ui->trust_level->addItem("Manually verify fingerprints", QVariant::fromValue<int>(SslTrust::TrustNoOne));

    this->ui->trusted_hosts->setModel(&this->current_trust.trusted_hosts);

    this->ui->trusted_hosts->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->ui->trusted_hosts->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    this->ui->trusted_hosts->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    connect(
        this->ui->trusted_hosts->selectionModel(),
        &QItemSelectionModel::currentChanged,
        this,
        &SslTrustEditor::on_trusted_server_selection);
}

SslTrustEditor::~SslTrustEditor()
{
    delete ui;
}

SslTrust SslTrustEditor::trust() const
{
    return this->current_trust;
}

void SslTrustEditor::setTrust(const SslTrust &trust)
{
    this->current_trust = trust;

    this->ui->trust_level->setCurrentIndex(
        this->ui->trust_level->findData(QVariant::fromValue<int>(trust.trust_level))
    );

    if(trust.enable_ca)
        this->ui->trust_enable_ca->setChecked(true);
    else
        this->ui->trust_disable__ca->setChecked(true);

    this->ui->trusted_hosts->resizeColumnsToContents();
}

void SslTrustEditor::on_trust_revoke_selected_clicked()
{
    this->current_trust.trusted_hosts.remove(this->ui->trusted_hosts->currentIndex());
}

void SslTrustEditor::on_trust_enable_ca_clicked()
{
    this->current_trust.enable_ca = true;
}

void SslTrustEditor::on_trust_disable__ca_clicked()
{
    this->current_trust.enable_ca = false;
}

void SslTrustEditor::on_trust_level_currentIndexChanged(int index)
{
    this->current_trust.trust_level = SslTrust::TrustLevel(this->ui->trust_level->itemData(index).toInt());
}

void SslTrustEditor::on_trusted_server_selection(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    if(auto host = this->current_trust.trusted_hosts.get(current); host) {
        this->ui->trust_revoke_selected->setEnabled(true);
    } else {
        this->ui->trust_revoke_selected->setEnabled(false);
    }
}
