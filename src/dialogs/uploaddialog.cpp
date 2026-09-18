#include "uploaddialog.hpp"
#include "ui_uploaddialog.h"
#include "ioutil.hpp"
#include "kristall.hpp"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMimeData>
#include <QMimeDatabase>
#include <QPushButton>

UploadDialog::UploadDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UploadDialog)
{
    ui->setupUi(this);

    connect( // connect with "this" as context, so the connection will die when the window is destroyed
        kristall::globals().localization.get(), &Localization::translationChanged,
        this, [this]() { this->ui->retranslateUi(this); },
        Qt::DirectConnection
    );

    this->ui->mime_type->setText("text/gemini");
    this->ui->text_input->setFont(kristall::globals().document_style.preformatted_font);

    this->updateUI();
}

UploadDialog::~UploadDialog()
{
    delete ui;
}

void UploadDialog::setCurrentPage(QByteArray const & data, QString const & mime)
{
    this->page_data = data;
    this->page_mime = mime;
}

QByteArray UploadDialog::data() const
{
    if(this->ui->text_mode->isChecked())
        return this->ui->text_input->toPlainText().toUtf8();

    QFile file { this->ui->file_name->text() };
    if(not file.open(QIODevice::ReadOnly))
        return QByteArray { };
    return file.readAll();
}

QString UploadDialog::mimeType() const
{
    return this->ui->mime_type->text();
}

QString UploadDialog::token() const
{
    return this->ui->token->text();
}

void UploadDialog::on_text_mode_toggled(bool checked)
{
    this->ui->input_stack->setCurrentIndex(checked ? 0 : 1);
    this->ui->text_page->setSizePolicy(QSizePolicy::Preferred, checked ? QSizePolicy::Expanding : QSizePolicy::Ignored);
    this->ui->file_page->setSizePolicy(QSizePolicy::Preferred, checked ? QSizePolicy::Ignored : QSizePolicy::Preferred);
    this->ui->input_stack->updateGeometry();
    this->layout()->activate();
    this->resize(this->width(), this->sizeHint().height());
    this->updateUI();
}

void UploadDialog::on_insert_page_button_clicked()
{
    this->ui->text_mode->setChecked(true);
    this->ui->text_input->setPlainText(QString::fromUtf8(this->page_data));
    this->ui->mime_type->setText(this->page_mime);
}

void UploadDialog::on_select_file_button_clicked()
{
    QFileDialog dialog { this };

    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.selectFile(this->ui->file_name->text());

    if(dialog.exec() != QDialog::Accepted)
        return;

    this->ui->file_name->setText(dialog.selectedFiles().constFirst());
}

void UploadDialog::on_file_name_textChanged(const QString &arg1)
{
    QMimeDatabase mime_db;
    this->ui->mime_type->setText(mime_db.mimeTypeForFile(arg1).name());
    this->updateUI();
}

void UploadDialog::on_text_input_textChanged()
{
    this->updateUI();
}

void UploadDialog::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void UploadDialog::dropEvent(QDropEvent *event)
{
    for (const auto &url : event->mimeData()->urls())
    {
        if (not url.isLocalFile())
            continue;
        this->ui->file_mode->setChecked(true);
        this->ui->file_name->setText(url.toLocalFile());
        break;
    }
}

void UploadDialog::updateUI()
{
    bool ok = true;
    qint64 size = 0;

    if(this->ui->text_mode->isChecked()) {
        size = this->ui->text_input->toPlainText().toUtf8().size();
    } else {
        QFileInfo info { this->ui->file_name->text() };
        ok &= info.isFile();
        size = info.size();
    }

    this->ui->size_label->setText(IoUtil::size_human(size));
    this->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ok);
}
