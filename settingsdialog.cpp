#include "settingsdialog.hpp"
#include "ui_settingsdialog.h"
#include <QFontDialog>
#include <QColorDialog>
#include <QStyle>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    current_style()
{
    ui->setupUi(this);

    static_assert(GeminiStyle::Fixed == 0);
    static_assert(GeminiStyle::AutoDarkTheme == 1);
    static_assert(GeminiStyle::AutoLightTheme == 2);

    this->ui->auto_theme->clear();
    this->ui->auto_theme->addItem("Disabled", QVariant::fromValue<int>(GeminiStyle::Fixed));
    this->ui->auto_theme->addItem("Dark Theme", QVariant::fromValue<int>(GeminiStyle::AutoDarkTheme));
    this->ui->auto_theme->addItem("Light Theme", QVariant::fromValue<int>(GeminiStyle::AutoLightTheme));

    setGeminiStyle(GeminiStyle { });
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

void SettingsDialog::setGeminiStyle(const GeminiStyle &style)
{
    static const QString COLOR_STYLE("border: 1px solid black; padding: 4px; background-color : %1; color : %2;");

    this->current_style = style;

    this->ui->auto_theme->setCurrentIndex(this->current_style.theme);

    this->ui->page_margin->setValue(this->current_style.margin);

    auto setFontAndColor = [this](QLabel * label, QFont font, QColor color)
    {
        label->setText(formatFont(font));
        label->setStyleSheet(COLOR_STYLE
            .arg(this->current_style.background_color.name())
            .arg(color.name()));
    };

    ui->bg_preview->setStyleSheet(COLOR_STYLE
        .arg(this->current_style.background_color.name())
        .arg("#FF00FF"));

    ui->link_local_preview->setStyleSheet(COLOR_STYLE
        .arg(this->current_style.background_color.name())
        .arg(this->current_style.internal_link_color.name()));

    ui->link_foreign_preview->setStyleSheet(COLOR_STYLE
        .arg(this->current_style.background_color.name())
        .arg(this->current_style.external_link_color.name()));

    ui->link_cross_preview->setStyleSheet(COLOR_STYLE
        .arg(this->current_style.background_color.name())
        .arg(this->current_style.cross_scheme_link_color.name()));

    setFontAndColor(this->ui->std_preview, this->current_style.standard_font, this->current_style.standard_color);
    setFontAndColor(this->ui->pre_preview, this->current_style.preformatted_font, this->current_style.preformatted_color);
    setFontAndColor(this->ui->h1_preview, this->current_style.h1_font, this->current_style.h1_color);
    setFontAndColor(this->ui->h2_preview, this->current_style.h2_font, this->current_style.h2_color);
    setFontAndColor(this->ui->h3_preview, this->current_style.h3_font, this->current_style.h3_color);

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

    QString host = this->ui->preview_url->text();
    if(host.length() == 0)
        host = "preview";

    DocumentOutlineModel outline;
    auto doc = GeminiRenderer { current_style }.render(
        document,
        QUrl(QString("about://%1/foobar").arg(host)),
        outline
    );

    ui->style_preview->setStyleSheet(QString("QTextBrowser { background-color: %1; }")
                                     .arg(doc->background_color.name()));
    ui->style_preview->setDocument(doc.get());
    preview_document = std::move(doc);
}

void SettingsDialog::updateFont(QFont & input)
{
    QFontDialog dialog { this };

    dialog.setCurrentFont(input);

    if(dialog.exec() == QDialog::Accepted) {
        input = dialog.currentFont();
        setGeminiStyle(current_style);
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

void SettingsDialog::updateColor(QColor &input)
{
    QColorDialog dialog { this };

    dialog.setCurrentColor(input);

    if(dialog.exec() == QDialog::Accepted) {
        input = dialog.currentColor();
        setGeminiStyle(current_style);
    }
}

void SettingsDialog::on_std_change_color_clicked()
{
    updateColor(current_style.standard_color);
}

void SettingsDialog::on_pre_change_color_clicked()
{
    updateColor(current_style.preformatted_color);
}

void SettingsDialog::on_h1_change_color_clicked()
{
    updateColor(current_style.h1_color);
}

void SettingsDialog::on_h2_change_color_clicked()
{
    updateColor(current_style.h2_color);
}

void SettingsDialog::on_h3_change_color_clicked()
{
    updateColor(current_style.h3_color);
}

void SettingsDialog::on_bg_change_color_clicked()
{
    updateColor(current_style.background_color);
}

void SettingsDialog::on_link_local_change_color_clicked()
{
    updateColor(current_style.internal_link_color);
}

void SettingsDialog::on_link_foreign_change_color_clicked()
{
    updateColor(current_style.external_link_color);
}

void SettingsDialog::on_link_cross_change_color_clicked()
{
    updateColor(current_style.cross_scheme_link_color);
}

void SettingsDialog::on_link_local_prefix_textChanged(const QString &text)
{
    current_style.internal_link_prefix = text;
    reloadStylePreview();
}

void SettingsDialog::on_link_foreign_prefix_textChanged(const QString &text)
{
    current_style.external_link_prefix = text;
    reloadStylePreview();
}

void SettingsDialog::on_auto_theme_currentIndexChanged(int index)
{
    if(index >= 0) {
        current_style.theme = GeminiStyle::Theme(index);
        reloadStylePreview();
    }
}

void SettingsDialog::on_preview_url_textChanged(const QString &arg1)
{
    this->reloadStylePreview();
}

void SettingsDialog::on_page_margin_valueChanged(double value)
{
    this->current_style.margin = value;
    this->reloadStylePreview();
}
