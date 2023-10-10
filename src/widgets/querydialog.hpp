#ifndef QUERYDIALOG_H
#define QUERYDIALOG_H

#include "ui_querydialog.h"
#include <QDialog>
#include <QLineEdit>
#include <QString>

class QueryDialog : public QDialog {
public:
    QueryDialog(QWidget *parent);
    void setLabelText(const QString &text);
    void setTextEchoMode(QLineEdit::EchoMode mode);
    QString textValue();
private:
    Ui_QueryDialog ui;
    QLineEdit::EchoMode mode;
};

#endif
