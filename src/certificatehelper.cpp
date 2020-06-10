#include "certificatehelper.hpp"

#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/pem.h>

CryptoIdentity CertificateHelper::createNewIdentity(const QString &common_name, QDateTime expiry_date)
{
    CryptoIdentity identity;

    auto const now = QDateTime::currentDateTime();
    if(expiry_date < now)
        return identity;

    identity.display_name = common_name;

    QByteArray common_name_utf8 = common_name.toUtf8();

    EVP_PKEY * pkey = EVP_PKEY_new();
    q_check_ptr(pkey);
    RSA * rsa = RSA_generate_key(2048, RSA_F4, nullptr, nullptr);
    q_check_ptr(rsa);
    EVP_PKEY_assign_RSA(pkey, rsa);
    X509 * x509 = X509_new();
    q_check_ptr(x509);
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
    X509_gmtime_adj(X509_get_notBefore(x509), 0); // not before current time
    X509_gmtime_adj(X509_get_notAfter(x509), now.secsTo(expiry_date)); // not after a year from this point
    X509_set_pubkey(x509, pkey);
    X509_NAME * name = X509_get_subject_name(x509);
    q_check_ptr(name);
    // X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char *)"US", -1, -1, 0);
    // X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char *)"My Organization", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, reinterpret_cast<unsigned char const *>(common_name_utf8.data()), common_name_utf8.size(), -1, 0);
    X509_set_issuer_name(x509, name);
    X509_sign(x509, pkey, EVP_sha1());
    BIO * bp_private = BIO_new(BIO_s_mem());
    q_check_ptr(bp_private);
    if(PEM_write_bio_PrivateKey(bp_private, pkey, nullptr, nullptr, 0, nullptr, nullptr) != 1)
    {
        EVP_PKEY_free(pkey);
        X509_free(x509);
        BIO_free_all(bp_private);
        qFatal("PEM_write_bio_PrivateKey");
    }
    BIO * bp_public = BIO_new(BIO_s_mem());
    q_check_ptr(bp_public);
    if(PEM_write_bio_X509(bp_public, x509) != 1)
    {
        EVP_PKEY_free(pkey);
        X509_free(x509);
        BIO_free_all(bp_public);
        BIO_free_all(bp_private);
        qFatal("PEM_write_bio_PrivateKey");
    }

    const char * buffer = nullptr;
    size_t size = BIO_get_mem_data(bp_public, &buffer);
    q_check_ptr(buffer);
    identity.certificate = QSslCertificate(QByteArray(buffer, size));
    if(identity.certificate.isNull())
    {
        qFatal("Failed to generate a random client certificate");
    }
    size = BIO_get_mem_data(bp_private, &buffer);
    q_check_ptr(buffer);
    identity.private_key = QSslKey(QByteArray(buffer, size), QSsl::Rsa);
    if(identity.private_key.isNull())
    {
        qFatal("Failed to generate a random private key");
    }

    EVP_PKEY_free(pkey); // this will also free the rsa key
    X509_free(x509);
    BIO_free_all(bp_public);
    BIO_free_all(bp_private);

    return identity;
}
