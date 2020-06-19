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

    //! If not empty, Kristall will check
    QString host_filter = "";

    //! When this is set to true and the host_filter is not empty,
    //! the certificate will be automatically enabled for hosts matching the filter.
    bool auto_enable = false;

    bool isValid() const {
        return (not this->certificate.isNull()) and (not this->private_key.isNull());
    }
};

#endif // CRYPTOIDENTITIY_HPP
