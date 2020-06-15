#include "ssltrust.hpp"

#include <QDebug>

void SslTrust::load(QSettings &settings)
{
    trust_level = TrustLevel(settings.value("trust_level", int(TrustOnFirstUse)).toInt());
    enable_ca = settings.value("enable_ca", QVariant::fromValue(false)).toBool();

    trusted_hosts.clear();

    int size = settings.beginReadArray("trusted_hosts");
    for(int i = 0; i < size; i++)
    {
        settings.setArrayIndex(i);

        auto key_type = QSsl::KeyAlgorithm(settings.value("key_type").toInt());
        auto key_value = settings.value("key_bits").toByteArray();

        TrustedHost host;
        host.host_name = settings.value("host_name").toString();
        host.trusted_at = settings.value("trusted_at").toDateTime();
        host.public_key = QSslKey(key_value, key_type, QSsl::Der, QSsl::PublicKey);

        trusted_hosts.insert(host);
    }
    settings.endArray();
}

void SslTrust::save(QSettings &settings) const
{
    settings.setValue("trust_level", int(trust_level));
    settings.setValue("enable_ca", enable_ca);

    auto all = trusted_hosts.getAll();
    settings.beginWriteArray("trusted_hosts", all.size());
    for(int i = 0; i < all.size(); i++)
    {
        settings.setArrayIndex(i);

        settings.setValue("host_name", all.at(i).host_name);
        settings.setValue("trusted_at", all.at(i).trusted_at);
        settings.setValue("key_type", int(all.at(i).public_key.algorithm()));
        settings.setValue("key_bits", all.at(i).public_key.toDer());
    }
    settings.endArray();
}

bool SslTrust::isTrusted(QUrl const & url, const QSslCertificate &certificate)
{
    if(trust_level == TrustEverything)
        return true;

    if(auto host_or_none = trusted_hosts.get(url.host()); host_or_none)
    {
        if(host_or_none->public_key == certificate.publicKey())
            return true;
        qDebug() << "certificate mismatch for" << url;
        return false;
    }
    else
    {
        if(trust_level == TrustOnFirstUse)
        {
            TrustedHost host;
            host.host_name = url.host();
            host.trusted_at = QDateTime::currentDateTime();
            host.public_key = certificate.publicKey();

            bool ok = trusted_hosts.insert(host);
            assert(ok);
            return true;
        }
        return false;
    }
}
