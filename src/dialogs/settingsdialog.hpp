#ifndef SETTINGSDIALOG_HPP
#define SETTINGSDIALOG_HPP

#include <QDialog>

#include "renderers/geminirenderer.hpp"
#include "protocolsetup.hpp"
#include "documentstyle.hpp"
#include "ssltrust.hpp"
#include "kristall.hpp"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    void setGeminiStyle(DocumentStyle const & style);

    DocumentStyle geminiStyle() const {
        return current_style;
    }

    ProtocolSetup protocols() const;
    void setProtocols(ProtocolSetup const & proto);

    SslTrust geminiSslTrust() const;
    void setGeminiSslTrust(SslTrust const & trust);

    SslTrust httpsSslTrust() const;
    void setHttpsSslTrust(SslTrust const & trust);

    GenericSettings options() const;
    void setOptions(GenericSettings const & options);

private slots:
    void on_std_change_font_clicked();

    void on_pre_change_font_clicked();

    void on_h1_change_font_clicked();

    void on_h2_change_font_clicked();

    void on_h3_change_font_clicked();

    void on_bq_change_font_clicked();

    void on_std_change_color_clicked();

    void on_pre_change_color_clicked();

    void on_h1_change_color_clicked();

    void on_h2_change_color_clicked();

    void on_h3_change_color_clicked();

    void on_bq_change_color_clicked();

    void on_bg_change_color_clicked();

    void on_link_local_change_color_clicked();

    void on_link_foreign_change_color_clicked();

    void on_link_cross_change_color_clicked();

    void on_link_local_prefix_textChanged(const QString &arg1);

    void on_link_foreign_prefix_textChanged(const QString &arg1);

    void on_auto_theme_currentIndexChanged(int index);

    void on_preview_url_textChanged(const QString &arg1);

    void on_page_margin_h_valueChanged(double arg1);
    void on_page_margin_v_valueChanged(double arg1);

    void on_enable_justify_text_clicked(bool arg1);

    void on_enable_text_width_clicked(bool arg1);

    void on_text_width_valueChanged(int value);

    void on_line_height_p_valueChanged(double arg1);

    void on_line_height_h_valueChanged(double arg1);

    void on_indent_bq_valueChanged(int value);
    void on_indent_p_valueChanged(int value);
    void on_indent_h_valueChanged(int value);
    void on_indent_l_valueChanged(int value);

    void on_indent_size_valueChanged(double value);

    void on_list_symbol_currentIndexChanged(int index);

    void on_presets_currentIndexChanged(int index);

    void on_preset_new_clicked();

    void on_SettingsDialog_accepted();

    void on_quote_change_color_clicked();

    void on_preset_save_clicked();

    void on_preset_load_clicked();

    void on_preset_import_clicked();

    void on_preset_export_clicked();

    void on_start_page_textChanged(const QString &arg1);

    void on_search_engine_currentTextChanged(const QString &arg1);

    void on_ui_theme_currentIndexChanged(int index);

    void on_icon_theme_currentIndexChanged(int index);

    void on_ui_density_currentIndexChanged(int index);

    void on_fancypants_on_clicked();
    void on_fancypants_off_clicked();

    void on_texthl_on_clicked();
    void on_texthl_off_clicked();

    void on_gophermap_icon_clicked();
    void on_gophermap_text_clicked();

    void on_scheme_os_default_clicked();
    void on_scheme_error_clicked();

    void on_show_hidden_files_clicked();
    void on_hide_hidden_files_clicked();

    void on_urlbarhl_fancy_clicked();
    void on_urlbarhl_none_clicked();

    void on_fancyquotes_on_clicked();
    void on_fancyquotes_off_clicked();

    void on_redirection_mode_currentIndexChanged(int index);

    void on_max_redirects_valueChanged(int arg1);

    void on_network_timeout_valueChanged(int arg1);

    void on_enable_home_btn_clicked(bool arg1);
    void on_enable_newtab_btn_clicked(bool arg1);
    void on_enable_root_btn_clicked(bool arg1);
    void on_enable_parent_btn_clicked(bool arg1);

    void on_cache_limit_valueChanged(int limit);
    void on_cache_threshold_valueChanged(int thres);
    void on_cache_life_valueChanged(int life);
    void on_enable_unlimited_cache_life_clicked(bool checked);

private:
    void reloadStylePreview();

    void updateFont(QFont & input);

    void updateColor(QColor & input);

private:
    Ui::SettingsDialog *ui;

    DocumentStyle current_style;
    std::unique_ptr<QTextDocument> preview_document;

    QMap<QString, DocumentStyle> predefined_styles;

    SslTrust current_trust;

    GenericSettings current_options;
};

#endif // SETTINGSDIALOG_HPP
