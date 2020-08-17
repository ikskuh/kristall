#ifndef SSLTRUSTEDITOR_HPP
#define SSLTRUSTEDITOR_HPP

#include <QWidget>
#include <QSortFilterProxyModel>

#include "ssltrust.hpp"

namespace Ui {
class SslTrustEditor;
}

class SslTrustEditor : public QWidget
{
    Q_OBJECT

public:
    explicit SslTrustEditor(QWidget *parent = nullptr);
    ~SslTrustEditor();


    SslTrust trust() const;
    void setTrust(SslTrust const & trust);

private slots:
    void on_trust_revoke_selected_clicked();

    void on_trust_enable_ca_clicked();

    void on_trust_disable__ca_clicked();

    void on_trust_level_currentIndexChanged(int index);

private:


    void on_trusted_server_selection(QModelIndex const & current, QModelIndex const & previous);

private:
    Ui::SslTrustEditor *ui;

    SslTrust current_trust;
};

#endif // SSLTRUSTEDITOR_HPP
