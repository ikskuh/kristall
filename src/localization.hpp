#ifndef LOCALIZATION_HPP
#define LOCALIZATION_HPP

#include <QObject>
#include <QTranslator>
#include <QLocale>
#include <optional>

class Localization : public QObject
{
    Q_OBJECT
public:
    explicit Localization(QObject *parent = nullptr);

    void setLocale(std::optional<QLocale> const & locale);

    void translate(QLocale const & locale);

signals:
    void translationChanged();

public:
    std::optional<QLocale> locale;
    QTranslator qt;
    QTranslator kristall;
};

#endif // LOCALIZATION_HPP
