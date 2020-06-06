#ifndef SETTINGSDIALOG_HPP
#define SETTINGSDIALOG_HPP

#include <QDialog>

#include "geminirenderer.hpp"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    void setStyle(GeminiStyle const & style);

private slots:
    void on_std_change_font_clicked();

    void on_pre_change_font_clicked();

    void on_h1_change_font_clicked();

    void on_h2_change_font_clicked();

    void on_h3_change_font_clicked();

private:
    void reloadStylePreview();

    void updateFont(QFont & input);

private:
    Ui::SettingsDialog *ui;

    GeminiStyle current_style;
    std::unique_ptr<QTextDocument> preview_document;
};

#endif // SETTINGSDIALOG_HPP
