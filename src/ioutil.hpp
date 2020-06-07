#ifndef IOUTIL_HPP
#define IOUTIL_HPP

#include <QIODevice>

struct IoUtil
{
    static bool writeAll(QIODevice & dst, QByteArray const & src);
};

#endif // IOUTIL_HPP
