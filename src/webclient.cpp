#include "webclient.hpp"
#include "kristall.hpp"

#include <QNetworkRequest>
#include <QNetworkReply>

WebClient::WebClient() :
    ProtocolHandler(nullptr),
    current_reply(nullptr)
{
    manager.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

WebClient::~WebClient()
{

}

bool WebClient::supportsScheme(const QString &scheme) const
{
    return (scheme == "https") or (scheme == "http");
}

bool WebClient::startRequest(const QUrl &url, RequestOptions options)
{
    if(url.scheme() != "http" and url.scheme() != "https")
        return false;

    if(this->current_reply != nullptr)
        return true;

    this->options = options;
    this->body.clear();

    QNetworkRequest request(url);

    auto ssl_config = request.sslConfiguration();
    // ssl_config.setProtocol(QSsl::TlsV1_2);
    if(global_https_trust.enable_ca)
        ssl_config.setCaCertificates(QSslConfiguration::systemCaCertificates());
    else
        ssl_config.setCaCertificates(QList<QSslCertificate> { });

    if(this->current_identity.isValid()) {
        ssl_config.setLocalCertificate(this->current_identity.certificate);
        ssl_config.setPrivateKey(this->current_identity.private_key);
    }

    // request.setMaximumRedirectsAllowed(5);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, false);
    request.setSslConfiguration(ssl_config);

    this->current_reply = manager.get(request);
    if(this->current_reply == nullptr)
        return false;

    this->suppress_socket_tls_error = true;

    connect(this->current_reply, &QNetworkReply::readyRead, this, &WebClient::on_data);
    connect(this->current_reply, &QNetworkReply::finished, this,  &WebClient::on_finished);
    connect(this->current_reply, &QNetworkReply::sslErrors, this, &WebClient::on_sslErrors);
    connect(this->current_reply, &QNetworkReply::redirected, this, &WebClient::on_redirected);

    return true;
}

bool WebClient::isInProgress() const
{
    return (this->current_reply != nullptr);
}

bool WebClient::cancelRequest()
{
    if(this->current_reply != nullptr)
    {
        this->current_reply->abort();
        this->current_reply = nullptr;
    }
    this->body.clear();
    return true;
}

bool WebClient::enableClientCertificate(const CryptoIdentity &ident)
{
    current_identity = ident;
    return true;
}

void WebClient::disableClientCertificate()
{
    current_identity = CryptoIdentity();
}

void WebClient::on_data()
{
    this->body.append(this->current_reply->readAll());
    emit this->requestProgress(this->body.size());
}

void WebClient::on_finished()
{
    auto * const reply = this->current_reply;
    this->current_reply = nullptr;

    reply->deleteLater();

    if(reply->error() != QNetworkReply::NoError)
    {
        NetworkError error = UnknownError;
        switch(reply->error())
        {
        case QNetworkReply::ConnectionRefusedError: error = ConnectionRefused; break;
        case QNetworkReply::RemoteHostClosedError: error = ProtocolViolation; break;
        case QNetworkReply::HostNotFoundError: error = HostNotFound; break;
        case QNetworkReply::TimeoutError: error = Timeout; break;
        case QNetworkReply::SslHandshakeFailedError: error = TlsFailure; break;

        case QNetworkReply::ContentAccessDenied: error = Unauthorized; break;
        case QNetworkReply::ContentOperationNotPermittedError: error = BadRequest; break;
        case QNetworkReply::ContentNotFoundError: error = ResourceNotFound; break;
        case QNetworkReply::AuthenticationRequiredError: error = Unauthorized; break;
        case QNetworkReply::ContentGoneError: error = ResourceNotFound; break;

        case QNetworkReply::InternalServerError: error = InternalServerError; break;
        case QNetworkReply::OperationNotImplementedError: error = InternalServerError; break;
        case QNetworkReply::ServiceUnavailableError: error = InternalServerError; break;
        default:
            qDebug() << "Unhandled server error:" << reply->error();
            break;
        }

        qDebug() << "web network error" << reply->errorString();
        qDebug() << this->body;

        if(not this->suppress_socket_tls_error) {
            emit this->networkError(error, reply->errorString());
        }
    }
    else
    {
        int statusCode =reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if(statusCode >= 200 and statusCode < 300) {
            auto mime = reply->header(QNetworkRequest::ContentTypeHeader).toString();
            emit this->requestComplete(this->body, mime);
        }
        else if(statusCode >= 300 and statusCode < 400) {
            auto url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

            emit this->redirected(url, (statusCode == 301) or (statusCode == 308));
        }
        else {
            emit networkError(UnknownError, QString("Unhandled HTTP status code %1").arg(statusCode));
        }

        this->body.clear();
    }
}

void WebClient::on_sslErrors(const QList<QSslError> &errors)
{
    if(options & IgnoreTlsErrors) {
        this->current_reply->ignoreSslErrors(errors);
        return;
    }

    QList<QSslError> remaining_errors = errors;
    QList<QSslError> ignored_errors;

    int i = 0;
    while(i < remaining_errors.size())
    {
        auto const & err = remaining_errors.at(i);

        bool ignore = false;
        if(SslTrust::isTrustRelated(err.error()))
        {
            switch(global_https_trust.getTrust(this->current_reply->url(), this->current_reply->sslConfiguration().peerCertificate()))
            {
            case SslTrust::Trusted:
                ignore = true;
                break;
            case SslTrust::Untrusted:
                this->suppress_socket_tls_error = true;
                emit this->networkError(UntrustedHost, "The requested host is not trusted.");
                return;
            case SslTrust::Mistrusted:
                this->suppress_socket_tls_error = true;
                emit this->networkError(MistrustedHost, "The requested is in the trust store and its signature changed..");
                return;
            }
        }
        else if(err.error() == QSslError::UnableToVerifyFirstCertificate)
        {
            ignore = true;
        }

        if(ignore) {
            ignored_errors.append(err);
            remaining_errors.removeAt(0);
        } else {
            i += 1;
        }
    }

    current_reply->ignoreSslErrors(ignored_errors);

    qDebug() << "ignoring" << ignored_errors.size() << "out of" << errors.size();

    for(auto const & error : remaining_errors) {
        qWarning() << int(error.error()) << error.errorString();
    }

    if(remaining_errors.size() > 0) {
        emit this->networkError(TlsFailure, remaining_errors.first().errorString());
    }
}

void WebClient::on_redirected(const QUrl &url)
{
    qDebug() << "redirected to" << url;
}
