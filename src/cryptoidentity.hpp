#ifndef CRYPTOIDENTITIY_HPP
#define CRYPTOIDENTITIY_HPP

#include <QObject>

#include <QSslCertificate>
#include <QSslKey>

//! Cryptographic user identitiy consisting
//! of a key-certificate pair and some user information.
struct CryptoIdentity
{
    //! The certificate that is used for cryptography
    QSslCertificate certificate;

    //! The actual private key that is used for cryptography
    QSslKey private_key;

    //! The title with which the identity is presented to the user.
    QString display_name;

    //! Notes that the user can have per identity for improved identity management
    QString user_notes;

    //! True for long-lived identities
    bool is_persistent = false;

    bool isValid() const {
        return (not this->certificate.isNull()) and (not this->private_key.isNull());
    }
};

#endif // CRYPTOIDENTITIY_HPP
