#include "cryptoidentity.hpp"

#include <QUrl>
#include <QRegularExpression>
#include <cassert>

static QRegularExpression fromWildcardCaseInsensitive(QStringView pattern)
{
    // Note: QRegularExpression converted from wildcard is anchored by default.
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    return QRegularExpression::fromWildcard(pattern, Qt::CaseInsensitive);
#else
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    QString strPattern = QRegularExpression::wildcardToRegularExpression(pattern);
#else
    QString strPattern = QRegularExpression::wildcardToRegularExpression(pattern.toString());
#endif
    return QRegularExpression(strPattern, QRegularExpression::CaseInsensitiveOption);
#endif
}

bool CryptoIdentity::isHostFiltered(const QUrl &url) const
{
    if(this->host_filter.isEmpty())
        return false;

    QString url_text = url.toString(QUrl::FullyEncoded);

    QRegularExpression pattern = fromWildcardCaseInsensitive(this->host_filter);

    return not pattern.match(url_text).hasMatch();
}

bool CryptoIdentity::isAutomaticallyEnabledOn(const QUrl &url) const
{
    if(this->host_filter.isEmpty())
        return false;
    if(not this->auto_enable)
        return false;

    QString url_text = url.toString(QUrl::FullyEncoded);

    QRegularExpression pattern = fromWildcardCaseInsensitive(this->host_filter);

    return pattern.match(url_text).hasMatch();
}
