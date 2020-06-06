#include "settingsdialog.hpp"
#include "ui_settingsdialog.h"
#include <QFontDialog>
#include <QColorDialog>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    current_style()
{
    ui->setupUi(this);

    setStyle(GeminiStyle { });
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

static QString formatFont(QFont const & font)
{
    QString style;
    if(font.italic() and font.bold())
        style = "bold, italic";
    else if(font.italic())
        style = "italic";
    else if(font.bold())
        style = "bold";
    else
        style = "regular";

    return QString("%1 (%2pt, %3)")
        .arg(font.family())
        .arg(font.pointSizeF())
        .arg(style);
}

void SettingsDialog::setStyle(const GeminiStyle &style)
{
    this->current_style = style;

    this->ui->std_preview->setText(formatFont(this->current_style.standard_font));
    this->ui->pre_preview->setText(formatFont(this->current_style.preformatted_font));
    this->ui->h1_preview->setText(formatFont(this->current_style.h1_font));
    this->ui->h2_preview->setText(formatFont(this->current_style.h2_font));
    this->ui->h3_preview->setText(formatFont(this->current_style.h3_font));

    this->reloadStylePreview();
}

void SettingsDialog::reloadStylePreview()
{
    auto const document = R"gemini(# H1 Header
## H2 Header
### H3 Header
Plain text document here.
* List A
* List B
=> rela-link Same-Site Link
=> //foreign.host/ Foreign Site Link
=> https://foreign.host/ Cross-Protocol Link
```
  ▄▄▄       ██▀███  ▄▄▄█████▓
 ▒████▄    ▓██ ▒ ██▒▓  ██▒ ▓▒
 ▒██  ▀█▄  ▓██ ░▄█ ▒▒ ▓██░ ▒░
 ░██▄▄▄▄██ ▒██▀▀█▄  ░ ▓██▓ ░
  ▓█   ▓██▒░██▓ ▒██▒  ▒██▒ ░
  ▒▒   ▓▒█░░ ▒▓ ░▒▓░  ▒ ░░
   ▒   ▒▒ ░  ░▒ ░ ▒░    ░
   ░   ▒     ░░   ░   ░
       ░  ░   ░
)gemini";


    DocumentOutlineModel outline;
    auto doc = GeminiRenderer { current_style }.render(
        document,
        QUrl("about://preview"),
        outline
    );

    ui->style_preview->setDocument(doc.get());
    preview_document = std::move(doc);
}

void SettingsDialog::updateFont(QFont & input)
{
    QFontDialog dialog { this };

    dialog.setCurrentFont(input);

    if(dialog.exec() == QDialog::Accepted) {
        input = dialog.currentFont();
        setStyle(current_style);
    }
}


void SettingsDialog::on_std_change_font_clicked()
{
    updateFont(current_style.standard_font);
}

void SettingsDialog::on_pre_change_font_clicked()
{
    updateFont(current_style.preformatted_font);
}

void SettingsDialog::on_h1_change_font_clicked()
{
    updateFont(current_style.h1_font);
}

void SettingsDialog::on_h2_change_font_clicked()
{
    updateFont(current_style.h2_font);
}

void SettingsDialog::on_h3_change_font_clicked()
{
    updateFont(current_style.h3_font);
}
