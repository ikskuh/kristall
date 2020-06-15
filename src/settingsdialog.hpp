#ifndef SETTINGSDIALOG_HPP
#define SETTINGSDIALOG_HPP

#include <QDialog>

#include "geminirenderer.hpp"
#include "protocolsetup.hpp"
#include "documentstyle.hpp"
#include "ssltrust.hpp"

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

    QUrl startPage() const;
    void setStartPage(QUrl const & url);

    ProtocolSetup protocols() const;
    void setProtocols(ProtocolSetup const & proto);

    QString uiTheme() const;
    void setUiTheme(QString const & theme);

    SslTrust sslTrust() const;
    void setSslTrust(SslTrust const & trust);

private slots:
    void on_std_change_font_clicked();

    void on_pre_change_font_clicked();

    void on_h1_change_font_clicked();

    void on_h2_change_font_clicked();

    void on_h3_change_font_clicked();

    void on_std_change_color_clicked();

    void on_pre_change_color_clicked();

    void on_h1_change_color_clicked();

    void on_h2_change_color_clicked();

    void on_h3_change_color_clicked();

    void on_bg_change_color_clicked();

    void on_link_local_change_color_clicked();

    void on_link_foreign_change_color_clicked();

    void on_link_cross_change_color_clicked();

    void on_link_local_prefix_textChanged(const QString &arg1);

    void on_link_foreign_prefix_textChanged(const QString &arg1);

    void on_auto_theme_currentIndexChanged(int index);

    void on_preview_url_textChanged(const QString &arg1);

    void on_page_margin_valueChanged(double arg1);

    void on_presets_currentIndexChanged(int index);

    void on_preset_new_clicked();

    void on_SettingsDialog_accepted();

    void on_quote_change_color_clicked();

    void on_preset_save_clicked();

    void on_preset_load_clicked();

    void on_preset_import_clicked();

    void on_preset_export_clicked();

    void on_trust_enable_ca_clicked();

    void on_trust_disable__ca_clicked();

    void on_trust_level_currentIndexChanged(int index);

    void on_trust_revoke_selected_clicked();

private:
    void reloadStylePreview();

    void updateFont(QFont & input);

    void updateColor(QColor & input);

    void on_trusted_server_selection(QModelIndex const & current, QModelIndex const & previous);

private:
    Ui::SettingsDialog *ui;

    DocumentStyle current_style;
    std::unique_ptr<QTextDocument> preview_document;

    QMap<QString, DocumentStyle> predefined_styles;

    SslTrust current_trust;
};

#endif // SETTINGSDIALOG_HPP
