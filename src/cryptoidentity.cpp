#include "cryptoidentity.hpp"

#include <QUrl>
#include <QRegExp>
#include <cassert>

bool CryptoIdentity::isHostFiltered(const QUrl &url) const
{
    if(this->host_filter.isEmpty())
        return false;

    QString url_text = url.toString(QUrl::FullyEncoded);

    QRegExp pattern { this->host_filter, Qt::CaseInsensitive, QRegExp::Wildcard };

    return not pattern.exactMatch(url_text);
}

bool CryptoIdentity::isAutomaticallyEnabledOn(const QUrl &url) const
{
    if(this->host_filter.isEmpty())
        return false;
    if(not this->auto_enable)
        return false;

    QString url_text = url.toString(QUrl::FullyEncoded);

    QRegExp pattern { this->host_filter, Qt::CaseInsensitive, QRegExp::Wildcard };

    return pattern.exactMatch(url_text);
}
