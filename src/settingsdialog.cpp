#include "settingsdialog.hpp"
#include "ui_settingsdialog.h"
#include <QFontDialog>
#include <QColorDialog>
#include <QStyle>
#include <QSettings>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

#include "kristall.hpp"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    current_style()
{
    ui->setupUi(this);

    static_assert(DocumentStyle::Fixed == 0);
    static_assert(DocumentStyle::AutoDarkTheme == 1);
    static_assert(DocumentStyle::AutoLightTheme == 2);

    this->ui->auto_theme->clear();
    this->ui->auto_theme->addItem("Disabled", QVariant::fromValue<int>(DocumentStyle::Fixed));
    this->ui->auto_theme->addItem("Dark Theme", QVariant::fromValue<int>(DocumentStyle::AutoDarkTheme));
    this->ui->auto_theme->addItem("Light Theme", QVariant::fromValue<int>(DocumentStyle::AutoLightTheme));

    this->ui->ui_theme->clear();
    this->ui->ui_theme->addItem("Light", QVariant::fromValue<QString>("light"));
    this->ui->ui_theme->addItem("Dark", QVariant::fromValue<QString>("dark"));

    setGeminiStyle(DocumentStyle { });

    if(global_settings.value("gophermap_display").toString() == "text") {
        this->ui->gophermap_text->setChecked(true);
    } else {
        this->ui->gophermap_icon->setChecked(true);
    }

    if(global_settings.value("text_display").toString() == "plain") {
        this->ui->fancypants_off->setChecked(true);
    } else {
        this->ui->fancypants_on->setChecked(true);
    }

    if(global_settings.value("text_decoration").toBool()) {
        this->ui->texthl_on->setChecked(true);
    } else {
        this->ui->texthl_off->setChecked(true);
    }

    if(global_settings.value("use_os_scheme_handler").toBool()) {
        this->ui->scheme_os_default->setChecked(true);
    } else {
        this->ui->scheme_error->setChecked(true);
    }

    int items = global_settings.beginReadArray("Themes");

    this->predefined_styles.clear();
    for(int i = 0; i < items; i++)
    {
        global_settings.setArrayIndex(i);

        QString name = global_settings.value("name").toString();

        DocumentStyle style;
        style.load(global_settings);

        this->predefined_styles.insert(name, style);
    }

    global_settings.endArray();

    this->ui->presets->clear();
    for(auto const & style_name : this->predefined_styles.keys())
    {
        this->ui->presets->addItem(style_name);
    }

    if(items > 0) {
        on_presets_currentIndexChanged(0);
    } else {
        this->on_presets_currentIndexChanged(-1);
    }

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
        &SettingsDialog::on_trusted_server_selection);

    this->ui->redirection_mode->clear();
    this->ui->redirection_mode->addItem("Ask for cross-scheme or cross-host redirection");
    this->ui->redirection_mode->addItem("Ask for cross-scheme redirection");
    this->ui->redirection_mode->addItem("Ask for cross-host redirection");
    this->ui->redirection_mode->addItem("Ask for all redirection");
    this->ui->redirection_mode->addItem("Silently redirect everything");
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

void SettingsDialog::setGeminiStyle(DocumentStyle const &style)
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

    ui->quote_preview->setStyleSheet(COLOR_STYLE
        .arg(this->current_style.blockquote_color.name())
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

QUrl SettingsDialog::startPage() const
{
    return QUrl(this->ui->start_page->text());
}

void SettingsDialog::setStartPage(const QUrl &url)
{
    this->ui->start_page->setText(url.toString());
}

ProtocolSetup SettingsDialog::protocols() const
{
    ProtocolSetup protocols;
#define M(X) \
    protocols.X   = this->ui->enable_##X->isChecked();
    PROTOCOLS(M)
#undef M
    return protocols;
}

void SettingsDialog::setProtocols(ProtocolSetup const & protocols)
{
#define M(X) \
    this->ui->enable_##X->setChecked(protocols.X);
    PROTOCOLS(M)
        #undef M
}

QString SettingsDialog::uiTheme() const
{
    return this->ui->ui_theme->currentData().toString();
}

void SettingsDialog::setUiTheme(const QString &theme)
{
    if(theme == "light")
        this->ui->ui_theme->setCurrentIndex(0);
    else if(theme == "dark")
        this->ui->ui_theme->setCurrentIndex(1);
    else
        this->ui->ui_theme->setCurrentIndex(0);

}

SslTrust SettingsDialog::sslTrust() const
{
    return this->current_trust;
}

void SettingsDialog::setSslTrust(const SslTrust &trust)
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

void SettingsDialog::reloadStylePreview()
{
    QFile document_src { ":/about/style-preview.gemini" };
    bool ok = document_src.open(QFile::ReadOnly);
    assert(ok and "failed to find style-preview.gemini!");

    auto const document = document_src.readAll();

    QString host = this->ui->preview_url->text();
    if(host.length() == 0)
        host = "preview";

    QUrl url { QUrl(QString("about://%1/foobar").arg(host)) };

    DocumentOutlineModel outline;
    auto doc = GeminiRenderer::render(
        document,
        url,
        current_style.derive(url),
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

void SettingsDialog::on_trusted_server_selection(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    if(auto host = this->current_trust.trusted_hosts.get(current); host) {
        this->ui->trust_revoke_selected->setEnabled(true);
    } else {
        this->ui->trust_revoke_selected->setEnabled(false);
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
void SettingsDialog::on_quote_change_color_clicked()
{
    updateColor(current_style.blockquote_color);
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
        current_style.theme = DocumentStyle::Theme(index);
        reloadStylePreview();
    }
}

void SettingsDialog::on_preview_url_textChanged(const QString &)
{
    this->reloadStylePreview();
}

void SettingsDialog::on_page_margin_valueChanged(double value)
{
    this->current_style.margin = value;
    this->reloadStylePreview();
}

void SettingsDialog::on_presets_currentIndexChanged(int index)
{
    this->ui->preset_load->setEnabled(index >= 0);
    this->ui->preset_save->setEnabled(index >= 0);
    this->ui->preset_export->setEnabled(index >= 0);
}

void SettingsDialog::on_preset_new_clicked()
{
    QInputDialog dlg { this };
    dlg.setInputMode(QInputDialog::TextInput);
    dlg.setOkButtonText("Save");
    dlg.setCancelButtonText("Cancel");
    dlg.setLabelText("Enter the name of your new preset:");

    if(dlg.exec() != QInputDialog::Accepted)
        return;
    QString name = dlg.textValue();

    bool override = false;
    if(this->predefined_styles.contains(name))
    {
        auto response = QMessageBox::question(this, "Kristall", QString("A style with the name '%1' already exists! Replace?").arg(name));
        if(response != QMessageBox::Yes)
            return;
        override = true;
    }

    this->predefined_styles.insert(name, this->current_style);

    if(not override)
    {
        this->ui->presets->addItem(name);
    }
}

void SettingsDialog::on_preset_save_clicked()
{
    QString name = this->ui->presets->currentText();
    if(name.isEmpty())
        return;

    auto response = QMessageBox::question(this, "Kristall", QString("Do you want to override the style '%1'?").arg(name));
    if(response != QMessageBox::Yes)
        return;

    this->predefined_styles.insert(name, this->current_style);
}


void SettingsDialog::on_preset_load_clicked()
{
    QString name = this->ui->presets->currentText();
    if(name.isEmpty())
        return;

    auto response = QMessageBox::question(this, "Kristall", QString("Do you want to load the style '%1'?\r\nThis will discard all currently set up values!").arg(name));
    if(response != QMessageBox::Yes)
        return;

    this->setGeminiStyle(this->predefined_styles.value(name));
}


void SettingsDialog::on_SettingsDialog_accepted()
{
    global_settings.setValue("gophermap_display", this->ui->gophermap_text->isChecked() ? "text" : "rendered");
    global_settings.setValue("text_display", this->ui->fancypants_off->isChecked() ? "plain" : "fancy");
    global_settings.setValue("text_decoration", this->ui->texthl_on->isChecked());
    global_settings.setValue("use_os_scheme_handler", this->ui->scheme_os_default->isChecked());

    global_settings.beginWriteArray("Themes", this->predefined_styles.size());

    int index = 0;
    for(auto const & style_name : this->predefined_styles.keys())
    {
        global_settings.setArrayIndex(index);

        global_settings.setValue("name", style_name);
        this->predefined_styles.value(style_name).save(global_settings);

        index += 1;
    }
    global_settings.endArray();
}

void SettingsDialog::on_preset_import_clicked()
{
    QFileDialog dialog { this };
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.selectNameFilter("Kristall Theme (*.kthm)");

    if(dialog.exec() !=QFileDialog::Accepted)
        return;

    QString fileName = dialog.selectedFiles().at(0);

    QSettings import_settings { fileName, QSettings::IniFormat };

    QString name;

    name = import_settings.value("name").toString();

    while(name.isEmpty())
    {
        QInputDialog dlg { this };
        dlg.setInputMode(QInputDialog::TextInput);
        dlg.setOkButtonText("Save");
        dlg.setCancelButtonText("Cancel");
        dlg.setLabelText("Imported preset has no name.\r\nPlease enter a name for the preset:");
        if(dlg.exec() != QDialog::Accepted)
            return;
        name = dlg.textValue();
    }

    bool override = false;
    if(this->predefined_styles.contains(name))
    {
        auto response = QMessageBox::question(this, "Kristall", QString("Do you want to override the style '%1'?").arg(name));
        if(response != QMessageBox::Yes)
            return;
        override = true;
    }

    DocumentStyle style;
    style.load(import_settings);

    this->predefined_styles.insert(name, style);

    if(not override)
    {
        this->ui->presets->addItem(name);
    }
}

void SettingsDialog::on_preset_export_clicked()
{
    QString name = this->ui->presets->currentText();
    if(name.isEmpty())
        return;

    QFileDialog dialog { this };
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.selectNameFilter("Kristall Theme (*.kthm)");
    dialog.selectFile(QString("%1.kthm").arg(name));

    if(dialog.exec() !=QFileDialog::Accepted)
        return;

    QString fileName = dialog.selectedFiles().at(0);

    QSettings export_settings { fileName, QSettings::IniFormat };
    export_settings.setValue("name", name);
    this->predefined_styles.value(name).save(export_settings);
    export_settings.sync();
}

void SettingsDialog::on_trust_enable_ca_clicked()
{
    this->current_trust.enable_ca = true;
}

void SettingsDialog::on_trust_disable__ca_clicked()
{
    this->current_trust.enable_ca = false;
}

void SettingsDialog::on_trust_level_currentIndexChanged(int index)
{
    this->current_trust.trust_level = SslTrust::TrustLevel(this->ui->trust_level->itemData(index).toInt());
}

void SettingsDialog::on_trust_revoke_selected_clicked()
{
    this->current_trust.trusted_hosts.remove(this->ui->trusted_hosts->currentIndex());
}
