#ifndef MIMEPARSER_HPP
#define MIMEPARSER_HPP

#include <QMap>
#include <QString>

struct MimeType
{
    QString type;
    QString subtype;
    QMap<QString, QString> parameters;

    bool is(QString const & type) const;
    bool is(QString const & type, QString const & sub_type) const;

    QString parameter(QString const & param_name, QString const & default_value = QString { }) const;

    bool isValid() const {
        return not type.isEmpty();
    }

    QString toString() const;
};

struct MimeParser
{
    MimeParser() = delete;



    static MimeType parse(QString const & mime_text);
};

#endif // MIMEPARSER_HPP
