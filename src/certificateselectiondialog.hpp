#ifndef CERTIFICATESELECTIONDIALOG_HPP
#define CERTIFICATESELECTIONDIALOG_HPP

#include <QDialog>

#include "cryptoidentity.hpp"

namespace Ui {
class CertificateSelectionDialog;
}

class CertificateSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CertificateSelectionDialog(QWidget *parent = nullptr);
    ~CertificateSelectionDialog();

    void setServerQuery(QString const & query);

    CryptoIdentity identity() const;

private slots:
    void on_use_temp_cert_30m_clicked();

    void on_use_temp_cert_1h_clicked();

    void on_use_temp_cert_12h_clicked();

    void on_use_temp_cert_24h_clicked();

    void on_use_temp_cert_48h_clicked();

private:
    //! Creates an anonymous identity with a randomly chosen name that
    //! will time out on `timeout`, then accepts the dialog.
    void acceptTemporaryWithTimeout(QDateTime timeout);

private:
    Ui::CertificateSelectionDialog *ui;

    CryptoIdentity cryto_identity;
};

#endif // CERTIFICATESELECTIONDIALOG_HPP
