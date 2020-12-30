#ifndef CERTIFICATEHELPER_HPP
#define CERTIFICATEHELPER_HPP

#include "cryptoidentity.hpp"

struct CertificateHelper
{
    CertificateHelper() = delete;



    static CryptoIdentity createNewIdentity(
            QString const & common_name,
            QDateTime const & expiry_date);
};

#endif // CERTIFICATEHELPER_HPP
