#ifndef IOUTIL_HPP
#define IOUTIL_HPP

#include <QIODevice>

struct IoUtil
{
    static bool writeAll(QIODevice & dst, QByteArray const & src);

    static QString size_human(qint64 size);
};

#endif // IOUTIL_HPP
