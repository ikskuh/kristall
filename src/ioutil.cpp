#include "ioutil.hpp"

bool IoUtil::writeAll(QIODevice &dst, QByteArray const & src)
{
    qint64 offset = 0;

    while(offset < src.size())
    {
        qint64 len = dst.write(src.data() + offset, src.size() - offset);
        if(len == 0)
            return false;
        offset += len;
    }

    return true;
}
