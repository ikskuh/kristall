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

bool WebClient::startRequest(const QUrl &url)
{
    if(url.scheme() != "http" and url.scheme() != "https")
        return false;

    if(this->current_reply != nullptr)
        return true;

    this->body.clear();

    QSslConfiguration ssl_config;
    // ssl_config.setProtocol(QSsl::TlsV1_2);
    // if(global_trust.enable_ca)
    //     ssl_config.setCaCertificates(QSslConfiguration::systemCaCertificates());
    // else
    //     ssl_config.setCaCertificates(QList<QSslCertificate> { });

    QNetworkRequest request(url);
    request.setMaximumRedirectsAllowed(5);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    request.setSslConfiguration(ssl_config);

    this->current_reply = manager.get(request);
    if(this->current_reply == nullptr)
        return false;

    connect(this->current_reply, &QNetworkReply::readyRead, this, &WebClient::on_data);
    connect(this->current_reply, &QNetworkReply::finished, this,  &WebClient::on_finished);
    connect(this->current_reply, &QNetworkReply::sslErrors, this, &WebClient::on_sslErrors);

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

void WebClient::on_data()
{
    this->body.append(this->current_reply->readAll());
    emit this->requestProgress(this->body.size());
}

void WebClient::on_finished()
{
    if(this->current_reply->error() != QNetworkReply::NoError)
    {
        NetworkError error = UnknownError;
        switch(this->current_reply->error())
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
            qDebug() << "Unhandled server error:" << this->current_reply->error();
            break;
        }

        qDebug() << "web network error" << this->current_reply->errorString();
        emit this->networkError(error, this->current_reply->errorString());
    }
    else
    {
        auto mime = this->current_reply->header(QNetworkRequest::ContentTypeHeader).toString();

        emit this->requestComplete(this->body, mime);

        this->body.clear();
    }
    this->current_reply->deleteLater();
    this->current_reply = nullptr;
}

void WebClient::on_sslErrors(const QList<QSslError> &errors)
{
    qDebug() << "HTTP SSL Errors:";
    for(auto const & err : errors)
        qDebug() << err;
    this->current_reply->ignoreSslErrors();
}
