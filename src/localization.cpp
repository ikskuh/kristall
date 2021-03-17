#include "kristall.hpp"

#include <QDebug>
#include <QLibraryInfo>

#include "localization.hpp"

Localization::Localization(QObject *parent) : QObject(parent)
{

}

void Localization::setLocale(const std::optional<QLocale> &locale)
{
    this->locale = locale;

    if(this->locale != std::nullopt)
        this->translate(*this->locale);
    else
        this->translate(QLocale());
}

void Localization::translate(const QLocale &locale)
{
    this->qt.load(locale, "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    this->kristall.load(locale, "kristall", "_", ":/i18n");

    emit this->translationChanged();

    qDebug() << "new locale is" << locale.bcp47Name() << locale.nativeLanguageName();
}
