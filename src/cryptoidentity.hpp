#ifndef CRYPTOIDENTITIY_HPP
#define CRYPTOIDENTITIY_HPP

#include <QObject>

#include <QSslCertificate>
#include <QSslKey>

//! Cryptographic user identitiy consisting
//! of a key-certificate pair and some user information.
struct CryptoIdentity
{
    QSslCertificate certificate;
    QSslKey private_key;
    QString display_name;

    bool is_persistent = false;

    bool isValid() const {
        return (not this->certificate.isNull()) and (not this->private_key.isNull());
    }
};

#endif // CRYPTOIDENTITIY_HPP
