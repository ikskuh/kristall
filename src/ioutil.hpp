#ifndef IOUTIL_HPP
#define IOUTIL_HPP

#include <QIODevice>
#include <QUrl>

struct IoUtil
{
    static bool writeAll(QIODevice & dst, QByteArray const & src);

    static QString size_human(qint64 size);

    static QUrl uniformUrl(QUrl url);
    static QString uniformUrlString(QUrl url);
};

#endif // IOUTIL_HPP
