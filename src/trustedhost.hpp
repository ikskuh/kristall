#ifndef TRUSTEDHOST_HPP
#define TRUSTEDHOST_HPP

#include <QSslKey>
#include <QUrl>
#include <QDateTime>

struct TrustedHost
{
    QString host_name;
    QSslKey public_key;
    QDateTime trusted_at;
};

#endif // TRUSTEDHOST_HPP
