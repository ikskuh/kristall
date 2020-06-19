#ifndef SSLTRUST_HPP
#define SSLTRUST_HPP

#include <QSslCertificate>
#include <QSslKey>
#include <QSettings>
#include <QSslError>

#include "trustedhostcollection.hpp"

struct SslTrust
{
    enum TrustLevel {
        TrustOnFirstUse = 0, // default
        TrustEverything = 1, // not recommended
        TrustNoOne      = 2, // approve every fingerprint by hand
    };

    SslTrust() = default;
    SslTrust(SslTrust const &) = default;
    SslTrust(SslTrust &&) = default;

    SslTrust & operator=(SslTrust const &) = default;
    SslTrust & operator=(SslTrust &&) = default;

    TrustLevel trust_level = TrustOnFirstUse;

    TrustedHostCollection trusted_hosts;

    bool enable_ca = false;

    void load(QSettings & settings);
    void save(QSettings & settings) const;

    bool isTrusted(QUrl const & url, QSslCertificate const & certificate);

    static bool isTrustRelated(QSslError::SslError err);
};

#endif // SSLTRUST_HPP
