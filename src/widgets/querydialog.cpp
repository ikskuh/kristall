#include "widgets/querydialog.hpp"

QueryDialog::QueryDialog(QWidget *parent) :
    QDialog(parent),
    mode(QLineEdit::Normal)
{
    ui.setupUi(this);
    ui.lineEdit->setVisible(false);
}

void QueryDialog::setLabelText(const QString &text)
{
    ui.query->setText(text);
}

void QueryDialog::setTextEchoMode(QLineEdit::EchoMode mode)
{
    ui.text->setVisible(mode == QLineEdit::Normal);
    ui.lineEdit->setVisible(mode != QLineEdit::Normal);
    ui.lineEdit->setEchoMode(mode);
    this->mode = mode;
}

QString QueryDialog::textValue()
{
    if (mode == QLineEdit::Normal)
        return ui.text->toPlainText();

    return ui.lineEdit->text();
}
