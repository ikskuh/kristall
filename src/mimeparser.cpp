#include "mimeparser.hpp"



bool MimeType::is(const QString &type) const
{
    return (this->type == type);
}

bool MimeType::is(const QString &type, const QString &sub_type) const
{
    return (this->type == type) and (this->subtype == sub_type);
}

QString MimeType::parameter(const QString &param_name, QString const & default_value) const
{
    auto val = parameters.value(param_name.toLower());
    if(val.isNull())
        val = default_value;
    return val;
}

QString MimeType::toString() const
{
    if(isValid()) {
        QString result = type;
        if(not subtype.isEmpty())
            result += "/" + subtype;
        for(auto const & key : parameters.keys()) {
            result += "; ";
            result += key;
            result += "=";
            result += parameters[key];
        }
        return result;
    } else {
        return QString { };
    }
}

MimeType MimeParser::parse(const QString &mime_text)
{
    MimeType type;

    QString arg_list;
    QString mime_part;

    if(int idx = mime_text.indexOf(';'); idx >= 0) {
        arg_list = mime_text.mid(idx + 1).trimmed().toLower();
        mime_part = mime_text.mid(0, idx).trimmed().toLower();
    } else {
        mime_part = mime_text.trimmed().toLower();
    }

    if(int idx = mime_part.indexOf('/'); idx >= 0) {
        type.type = mime_part.mid(0, idx);
        type.subtype = mime_part.mid(idx + 1);
    } else {
        type.type = mime_part;
        type.subtype = QString { };
    }

    if(not arg_list.isEmpty()) {
        for(auto const & _arg : arg_list.split(';'))
        {
            QString arg = _arg.trimmed();
            if(arg.isEmpty()) // skip over double spaces
                continue;

            QString key;
            QString value;

            if(int idx = arg.indexOf('='); idx >= 0) {
                key = arg.mid(0, idx);
                value = arg.mid(idx + 1);
            } else {
                key = arg;
                value = "";
            }

            type.parameters.insert(key, value);
        }
    }

    return type;
}
