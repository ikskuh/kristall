#include "settingsdialog.hpp"
#include "ui_settingsdialog.h"
#include <QFontDialog>
#include <QColorDialog>
#include <QStyle>
#include <QSettings>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QDebug>

#include <cassert>

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
    this->ui->auto_theme->addItem(tr("Disabled"), QVariant::fromValue<int>(DocumentStyle::Fixed));
    this->ui->auto_theme->addItem(tr("Dark Theme"), QVariant::fromValue<int>(DocumentStyle::AutoDarkTheme));
    this->ui->auto_theme->addItem(tr("Light Theme"), QVariant::fromValue<int>(DocumentStyle::AutoLightTheme));

    this->ui->ui_theme->clear();
    this->ui->ui_theme->addItem(tr("OS Default"), QVariant::fromValue<int>(int(Theme::os_default)));
    this->ui->ui_theme->addItem(tr("Light"), QVariant::fromValue<int>(int(Theme::light)));
    this->ui->ui_theme->addItem(tr("Dark"), QVariant::fromValue<int>(int(Theme::dark)));

    this->ui->icon_theme->clear();
    this->ui->icon_theme->addItem(tr("Automatic"), QVariant::fromValue<int>(int(IconTheme::automatic)));
    this->ui->icon_theme->addItem(tr("Light"), QVariant::fromValue<int>(int(IconTheme::light)));
    this->ui->icon_theme->addItem(tr("Dark"), QVariant::fromValue<int>(int(IconTheme::dark)));

    this->ui->ui_density->clear();
    this->ui->ui_density->addItem(tr("Compact"), QVariant::fromValue<int>(int(UIDensity::compact)));
    this->ui->ui_density->addItem(tr("Classic"), QVariant::fromValue<int>(int(UIDensity::classic)));

    this->ui->list_symbol->clear();
    this->ui->list_symbol->addItem(tr("Filled circle"), QVariant::fromValue<int>(int(QTextListFormat::Style::ListDisc)));
    this->ui->list_symbol->addItem(tr("Circle"), QVariant::fromValue<int>(int(QTextListFormat::Style::ListCircle)));
    this->ui->list_symbol->addItem(tr("Square"), QVariant::fromValue<int>(int(QTextListFormat::Style::ListSquare)));

    setGeminiStyle(DocumentStyle { });

    this->predefined_styles.clear();
    for(auto const & fileName : kristall::dirs::styles.entryList())
    {
        QSettings style_sheet {
            kristall::dirs::styles.absoluteFilePath(fileName),
            QSettings::IniFormat
        };

        QString name = style_sheet.value("name").toString();

        DocumentStyle style;
        style.load(style_sheet);

        this->predefined_styles.insert(name, style);
    }

    this->ui->presets->clear();
    for(auto const & style_name : this->predefined_styles.keys())
    {
        this->ui->presets->addItem(style_name);
    }

    if(this->predefined_styles.size() > 0) {
        on_presets_currentIndexChanged(0);
    } else {
        this->on_presets_currentIndexChanged(-1);
    }

    this->ui->redirection_mode->clear();
    this->ui->redirection_mode->addItem(tr("Ask for cross-scheme or cross-host redirection"), int(GenericSettings::WarnOnHostChange | GenericSettings::WarnOnSchemeChange));
    this->ui->redirection_mode->addItem(tr("Ask for cross-scheme redirection"), int(GenericSettings::WarnOnSchemeChange));
    this->ui->redirection_mode->addItem(tr("Ask for cross-host redirection"), int(GenericSettings::WarnOnHostChange));
    this->ui->redirection_mode->addItem(tr("Ask for all redirection"), int(GenericSettings::WarnAlways));
    this->ui->redirection_mode->addItem(tr("Silently redirect everything"), int(GenericSettings::WarnNever));

    connect(this->ui->tabWidget, &QTabWidget::currentChanged, this, [this] (int index) {
        if (index != 1) /* Style tab */
            return;

        this->ui->style_scroll_area->setMinimumWidth(this->ui->style_scroll_layout->minimumSize().width()
            + this->ui->style_scroll_area->verticalScrollBar()->sizeHint().width());
    });
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

    this->ui->link_local_prefix->setText(this->current_style.internal_link_prefix);
    this->ui->link_foreign_prefix->setText(this->current_style.external_link_prefix);

    this->ui->page_margin_h->setValue(this->current_style.margin_h);
    this->ui->page_margin_v->setValue(this->current_style.margin_v);

    this->ui->enable_justify_text->setChecked(this->current_style.justify_text);

    this->ui->enable_centre_h1->setChecked(this->current_style.centre_h1);

    this->ui->enable_text_width->setChecked(this->current_style.text_width_enabled);
    this->ui->text_width->setEnabled(this->current_style.text_width_enabled);
    this->ui->text_width->setValue(this->current_style.text_width);

    this->ui->line_height_p->setValue(this->current_style.line_height_p);
    this->ui->line_height_h->setValue(this->current_style.line_height_h);

    this->ui->indent_bq->setValue(this->current_style.indent_bq);
    this->ui->indent_p->setValue(this->current_style.indent_p);
    this->ui->indent_h->setValue(this->current_style.indent_h);
    this->ui->indent_l->setValue(this->current_style.indent_l);
    this->ui->indent_size->setValue(this->current_style.indent_size);

    this->ui->list_symbol->setCurrentIndex(0);
    for(int i = 0; i < this->ui->list_symbol->count(); ++i) {
        if(this->ui->list_symbol->itemData(i).toInt() == int(this->current_style.list_symbol)) {
            this->ui->list_symbol->setCurrentIndex(i);
            break;
        }
    }

    auto setFontAndColor = [this](QLabel * label, const QFont &font, QColor color)
    {
        label->setText(formatFont(font));
        label->setStyleSheet(COLOR_STYLE
            .arg(this->current_style.background_color.name(), color.name()));
    };

    ui->bg_preview->setStyleSheet(COLOR_STYLE
        .arg(this->current_style.background_color.name(), "#FF00FF"));

    ui->quote_preview->setStyleSheet(COLOR_STYLE
        .arg(this->current_style.blockquote_bgcolor.name(), "#FF00FF"));

    ui->link_local_preview->setStyleSheet(COLOR_STYLE
        .arg(this->current_style.background_color.name(), this->current_style.internal_link_color.name()));

    ui->link_foreign_preview->setStyleSheet(COLOR_STYLE
        .arg(this->current_style.background_color.name(), this->current_style.external_link_color.name()));

    ui->link_cross_preview->setStyleSheet(COLOR_STYLE
        .arg(this->current_style.background_color.name(), this->current_style.cross_scheme_link_color.name()));

    setFontAndColor(this->ui->std_preview, this->current_style.standard_font, this->current_style.standard_color);
    setFontAndColor(this->ui->pre_preview, this->current_style.preformatted_font, this->current_style.preformatted_color);
    setFontAndColor(this->ui->h1_preview, this->current_style.h1_font, this->current_style.h1_color);
    setFontAndColor(this->ui->h2_preview, this->current_style.h2_font, this->current_style.h2_color);
    setFontAndColor(this->ui->h3_preview, this->current_style.h3_font, this->current_style.h3_color);
    setFontAndColor(this->ui->bq_preview, this->current_style.blockquote_font, this->current_style.blockquote_fgcolor);

    this->reloadStylePreview();
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

SslTrust SettingsDialog::geminiSslTrust() const
{
    return this->ui->gemini_trust_editor->trust();
}

void SettingsDialog::setGeminiSslTrust(const SslTrust &trust)
{
    return this->ui->gemini_trust_editor->setTrust(trust);
}

SslTrust SettingsDialog::httpsSslTrust() const
{
    return this->ui->https_trust_editor->trust();
}

void SettingsDialog::setHttpsSslTrust(const SslTrust &trust)
{
    this->ui->https_trust_editor->setTrust(trust);
}

void SettingsDialog::setOptions(const GenericSettings &options)
{
    this->current_options = options;

    this->ui->ui_theme->setCurrentIndex(0);
    for(int i = 0; i < this->ui->ui_theme->count(); i++) {
        if(this->ui->ui_theme->itemData(i).toInt() == int(options.theme)) {
            this->ui->ui_theme->setCurrentIndex(i);
            break;
        }
    }

    this->ui->icon_theme->setCurrentIndex(0);
    for(int i = 0; i < this->ui->icon_theme->count(); i++) {
        if(this->ui->icon_theme->itemData(i).toInt() == int(options.icon_theme)) {
            this->ui->icon_theme->setCurrentIndex(i);
            break;
        }
    }


    this->ui->ui_density->setCurrentIndex(0);
    for(int i = 0; i < this->ui->ui_density->count(); ++i) {
        if (this->ui->ui_density->itemData(i).toInt() == int(options.ui_density)) {
            this->ui->ui_density->setCurrentIndex(i);
            break;
        }
    }

    this->ui->start_page->setText(this->current_options.start_page);

    this->ui->search_engine->clear();
    QString search = this->current_options.search_engine;
    this->ui->search_engine->lineEdit()->setPlaceholderText(tr("URL with '%1' in place of query"));
    this->ui->search_engine->addItem("gemini://geminispace.info/search?%1");
    this->ui->search_engine->addItem("gemini://gus.guru/search?%1");
    this->ui->search_engine->addItem("gemini://houston.coder.town/search?%1");
    this->ui->search_engine->addItem("gopher://gopher.floodgap.com:70/7/v2/vs?%1");
    this->ui->search_engine->setCurrentText(search);

    if(this->current_options.gophermap_display == GenericSettings::PlainText) {
        this->ui->gophermap_text->setChecked(true);
    } else {
        this->ui->gophermap_icon->setChecked(true);
    }

    if(this->current_options.text_display == GenericSettings::PlainText) {
        this->ui->fancypants_off->setChecked(true);
    } else {
        this->ui->fancypants_on->setChecked(true);
    }

    if(this->current_options.enable_text_decoration) {
        this->ui->texthl_on->setChecked(true);
    } else {
        this->ui->texthl_off->setChecked(true);
    }

    if(this->current_options.use_os_scheme_handler) {
        this->ui->scheme_os_default->setChecked(true);
    } else {
        this->ui->scheme_error->setChecked(true);
    }

    if(this->current_options.show_hidden_files_in_dirs) {
        this->ui->show_hidden_files->setChecked(true);
    } else {
        this->ui->hide_hidden_files->setChecked(true);
    }

    if(this->current_options.fancy_urlbar) {
        this->ui->urlbarhl_fancy->setChecked(true);
    } else {
        this->ui->urlbarhl_none->setChecked(true);
    }

    if (kristall::EMOJIS_SUPPORTED && this->current_options.emojis_enabled)
    {
        this->ui->emojis_on->setChecked(true);
    }
    else
    {
        this->ui->emojis_off->setChecked(true);

        // Grey out emoji options on unsupported emoji builds
        if (!kristall::EMOJIS_SUPPORTED)
        {
            this->ui->emojis_on->setEnabled(false);
            this->ui->emojis_off->setEnabled(false);
            this->ui->emojis_label->setToolTip(
                this->ui->emojis_label->toolTip() + tr(" (not supported in this build)"));
        }
    }

    if(this->current_options.fancy_quotes) {
        this->ui->fancyquotes_on->setChecked(true);
    } else {
        this->ui->fancyquotes_off->setChecked(true);
    }

    this->ui->max_redirects->setValue(this->current_options.max_redirections);

    this->ui->redirection_mode->setCurrentIndex(0);
    for(int i = 0; i < this->ui->redirection_mode->count(); i++)
    {
        if(this->ui->redirection_mode->itemData(i).toInt() == this->current_options.redirection_policy) {
            this->ui->redirection_mode->setCurrentIndex(i);
            break;
        }
    }

    this->ui->network_timeout->setValue(this->current_options.network_timeout);

    this->ui->enable_home_btn->setChecked(this->current_options.enable_home_btn);
    this->ui->enable_newtab_btn->setChecked(this->current_options.enable_newtab_btn);
    this->ui->enable_root_btn->setChecked(this->current_options.enable_root_btn);
    this->ui->enable_parent_btn->setChecked(this->current_options.enable_parent_btn);

    this->ui->cache_limit->setValue(this->current_options.cache_limit);
    this->ui->cache_threshold->setValue(this->current_options.cache_threshold);
    this->ui->cache_life->setValue(this->current_options.cache_life);
    this->ui->enable_unlimited_cache_life->setChecked(this->current_options.cache_unlimited_life);
    this->ui->cache_life->setEnabled(!this->current_options.cache_unlimited_life);
}

GenericSettings SettingsDialog::options() const
{
    return this->current_options;
}

void SettingsDialog::reloadStylePreview()
{
    QFile document_src { ":/about/style-display.gemini" };
    bool ok = document_src.open(QFile::ReadOnly);
    assert(ok and "failed to find style-display.gemini!");

    auto const document = document_src.readAll();

    QString host = this->ui->preview_url->text();
    if(host.length() == 0)
        host = "preview";

    QUrl url { QUrl(QString("about://%1/foobar").arg(host)) };

    DocumentOutlineModel outline;
    auto doc_style = current_style.derive(url);
    auto doc = GeminiRenderer::render(
        document,
        url,
        doc_style,
        outline
    );

    ui->style_preview->setStyleSheet(QString("QTextBrowser { background-color: %1; color: %2; }")
                                     .arg(doc_style.background_color.name(), doc_style.standard_color.name()));
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

void SettingsDialog::on_bq_change_font_clicked()
{
    updateFont(current_style.blockquote_font);
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

void SettingsDialog::on_bq_change_color_clicked()
{
    updateColor(current_style.blockquote_fgcolor);
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
    updateColor(current_style.blockquote_bgcolor);
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

void SettingsDialog::on_page_margin_h_valueChanged(double value)
{
    this->current_style.margin_h = value;
    this->reloadStylePreview();
}

void SettingsDialog::on_page_margin_v_valueChanged(double value)
{
    this->current_style.margin_v = value;
    this->reloadStylePreview();
}

void SettingsDialog::on_enable_justify_text_clicked(bool checked)
{
    this->current_style.justify_text = checked;
    this->reloadStylePreview();
}

void SettingsDialog::on_enable_centre_h1_clicked(bool checked)
{
    this->current_style.centre_h1 = checked;
    this->reloadStylePreview();
}

void SettingsDialog::on_enable_text_width_clicked(bool checked)
{
    this->current_style.text_width_enabled = checked;
    this->ui->text_width->setEnabled(checked);
}

void SettingsDialog::on_text_width_valueChanged(int value)
{
    this->current_style.text_width = value;
}

void SettingsDialog::on_line_height_p_valueChanged(double value)
{
    this->current_style.line_height_p = value;
    this->reloadStylePreview();
}

void SettingsDialog::on_line_height_h_valueChanged(double value)
{
    this->current_style.line_height_h = value;
    this->reloadStylePreview();
}

void SettingsDialog::on_indent_bq_valueChanged(int value)
{
    this->current_style.indent_bq = value;
    this->reloadStylePreview();
}

void SettingsDialog::on_indent_h_valueChanged(int value)
{
    this->current_style.indent_h = value;
    this->reloadStylePreview();
}

void SettingsDialog::on_indent_p_valueChanged(int value)
{
    this->current_style.indent_p = value;
    this->reloadStylePreview();
}

void SettingsDialog::on_indent_l_valueChanged(int value)
{
    this->current_style.indent_l = value;
    this->reloadStylePreview();
}

void SettingsDialog::on_indent_size_valueChanged(double value)
{
    this->current_style.indent_size = value;
    this->reloadStylePreview();
}

void SettingsDialog::on_list_symbol_currentIndexChanged(int index)
{
    if(index >= 0) {
        current_style.list_symbol = QTextListFormat::Style(this->ui->list_symbol->itemData(index).toInt());
        reloadStylePreview();
    }
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
    dlg.setOkButtonText(tr("Save"));
    dlg.setCancelButtonText(tr("Cancel"));
    dlg.setLabelText(tr("Enter the name of your new preset:"));

    if(dlg.exec() != QInputDialog::Accepted)
        return;
    QString name = dlg.textValue();

    bool override = false;
    if(this->predefined_styles.contains(name))
    {
        auto response = QMessageBox::question(this, "Kristall", QString(tr("A style with the name '%1' already exists! Replace?")).arg(name));
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

    auto response = QMessageBox::question(this, "Kristall", QString(tr("Do you want to override the style '%1'?")).arg(name));
    if(response != QMessageBox::Yes)
        return;

    this->predefined_styles.insert(name, this->current_style);
}


void SettingsDialog::on_preset_load_clicked()
{
    QString name = this->ui->presets->currentText();
    if(name.isEmpty())
        return;

    auto response = QMessageBox::question(this, "Kristall", QString(tr("Do you want to load the style '%1'?\r\nThis will discard all currently set up values!")).arg(name));
    if(response != QMessageBox::Yes)
        return;

    this->setGeminiStyle(this->predefined_styles.value(name));
}


void SettingsDialog::on_SettingsDialog_accepted()
{
    QStringList files = kristall::dirs::styles.entryList();

    for(auto const & style_name : this->predefined_styles.keys())
    {
        QString fileName = DocumentStyle::createFileNameFromName(style_name, 0);
        files.removeAll(fileName);

        QSettings style_sheet {
            kristall::dirs::styles.absoluteFilePath(fileName),
            QSettings::IniFormat
        };

        style_sheet.setValue("name", style_name);
        this->predefined_styles.value(style_name).save(style_sheet);

        style_sheet.sync();
    }

    for(auto const & fileName : files)
    {
        kristall::dirs::styles.remove(fileName);
    }
}

void SettingsDialog::on_preset_import_clicked()
{
    QFileDialog dialog { this };
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.selectNameFilter(tr("Kristall Theme (*.kthm)"));

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
        dlg.setOkButtonText(tr("Save"));
        dlg.setCancelButtonText(tr("Cancel"));
        dlg.setLabelText(tr("Imported preset has no name.\r\nPlease enter a name for the preset:"));
        if(dlg.exec() != QDialog::Accepted)
            return;
        name = dlg.textValue();
    }

    bool override = false;
    if(this->predefined_styles.contains(name))
    {
        auto response = QMessageBox::question(this, "Kristall", QString(tr("Do you want to override the style '%1'?")).arg(name));
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
    dialog.selectNameFilter(tr("Kristall Theme (*.kthm)"));
    dialog.selectFile(QString("%1.kthm").arg(name));

    if(dialog.exec() !=QFileDialog::Accepted)
        return;

    QString fileName = dialog.selectedFiles().at(0);

    QSettings export_settings { fileName, QSettings::IniFormat };
    export_settings.setValue("name", name);
    this->predefined_styles.value(name).save(export_settings);
    export_settings.sync();
}

void SettingsDialog::on_start_page_textChanged(const QString &start_page)
{
    this->current_options.start_page = start_page;
}

void SettingsDialog::on_search_engine_currentTextChanged(const QString &search_engine)
{
    this->current_options.search_engine = search_engine;
}

void SettingsDialog::on_ui_theme_currentIndexChanged(int index)
{
    this->current_options.theme = Theme(this->ui->ui_theme->itemData(index).toInt());

    kristall::setTheme(this->current_options.theme);
}

void SettingsDialog::on_icon_theme_currentIndexChanged(int index)
{
    this->current_options.icon_theme = IconTheme(this->ui->icon_theme->itemData(index).toInt());

    kristall::setIconTheme(this->current_options.icon_theme, this->current_options.theme);
}


void SettingsDialog::on_ui_density_currentIndexChanged(int index)
{
    this->current_options.ui_density = UIDensity(this->ui->ui_density->itemData(index).toInt());

    kristall::setUiDensity(this->current_options.ui_density, true);
}

void SettingsDialog::on_fancypants_on_clicked()
{
    this->current_options.text_display = GenericSettings::FormattedText;
}

void SettingsDialog::on_fancypants_off_clicked()
{
    this->current_options.text_display = GenericSettings::PlainText;
}

void SettingsDialog::on_texthl_on_clicked()
{
    this->current_options.enable_text_decoration = true;
}

void SettingsDialog::on_texthl_off_clicked()
{
    this->current_options.enable_text_decoration = false;
}

void SettingsDialog::on_gophermap_icon_clicked()
{
    this->current_options.gophermap_display = GenericSettings::FormattedText;
}

void SettingsDialog::on_gophermap_text_clicked()
{
    this->current_options.gophermap_display = GenericSettings::PlainText;
}

void SettingsDialog::on_scheme_os_default_clicked()
{
    this->current_options.use_os_scheme_handler = true;
}

void SettingsDialog::on_scheme_error_clicked()
{
    this->current_options.use_os_scheme_handler = false;
}

void SettingsDialog::on_show_hidden_files_clicked()
{
    this->current_options.show_hidden_files_in_dirs = true;
}

void SettingsDialog::on_hide_hidden_files_clicked()
{
    this->current_options.show_hidden_files_in_dirs = false;
}

void SettingsDialog::on_urlbarhl_fancy_clicked()
{
    this->current_options.fancy_urlbar = true;
}

void SettingsDialog::on_urlbarhl_none_clicked()
{
    this->current_options.fancy_urlbar = false;
}

void SettingsDialog::on_emojis_on_clicked()
{
    this->current_options.emojis_enabled = true;
}

void SettingsDialog::on_emojis_off_clicked()
{
    this->current_options.emojis_enabled = false;
}

void SettingsDialog::on_fancyquotes_on_clicked()
{
    this->current_options.fancy_quotes = true;
}

void SettingsDialog::on_fancyquotes_off_clicked()
{
    this->current_options.fancy_quotes = false;
}

void SettingsDialog::on_redirection_mode_currentIndexChanged(int index)
{
    this->current_options.redirection_policy = GenericSettings::RedirectionWarning(this->ui->redirection_mode->itemData(index).toInt());
}

void SettingsDialog::on_max_redirects_valueChanged(int max_redirections) {
    this->current_options.max_redirections = max_redirections;
}

void SettingsDialog::on_network_timeout_valueChanged(int timeout)
{
    this->current_options.network_timeout = timeout;
}

void SettingsDialog::on_enable_home_btn_clicked(bool checked)
{
    this->current_options.enable_home_btn = checked;
}

void SettingsDialog::on_enable_newtab_btn_clicked(bool checked)
{
    this->current_options.enable_newtab_btn = checked;
}

void SettingsDialog::on_enable_root_btn_clicked(bool checked)
{
    this->current_options.enable_root_btn = checked;
}

void SettingsDialog::on_enable_parent_btn_clicked(bool checked)
{
    this->current_options.enable_parent_btn = checked;
}

void SettingsDialog::on_cache_limit_valueChanged(int limit)
{
    this->current_options.cache_limit = limit;
}

void SettingsDialog::on_cache_threshold_valueChanged(int thres)
{
    this->current_options.cache_threshold = thres;
}

void SettingsDialog::on_cache_life_valueChanged(int life)
{
    this->current_options.cache_life = life;
}

void SettingsDialog::on_enable_unlimited_cache_life_clicked(bool checked)
{
    this->current_options.cache_unlimited_life = checked;
    this->ui->cache_life->setEnabled(!checked);
}
