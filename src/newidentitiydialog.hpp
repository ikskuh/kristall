#ifndef NEWIDENTITIYDIALOG_HPP
#define NEWIDENTITIYDIALOG_HPP

#include <QDialog>

#include "cryptoidentity.hpp"

namespace Ui {
class NewIdentitiyDialog;
}

class NewIdentitiyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewIdentitiyDialog(QWidget *parent = nullptr);
    ~NewIdentitiyDialog();

    //! Creates a new identity from the currently set
    //! user settings.
    CryptoIdentity createIdentity() const;

    QString groupName() const;
    void setGroupName(QString const & name);

private slots:
    void on_group_editTextChanged(const QString &arg1);

    void on_display_name_textChanged(const QString &arg1);

    void on_common_name_textChanged(const QString &arg1);

private:
    void updateUI();

private:
    Ui::NewIdentitiyDialog *ui;
};

#endif // NEWIDENTITIYDIALOG_HPP
