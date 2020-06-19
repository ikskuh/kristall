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

    QSslConfiguration ssl_config;
    // ssl_config.setProtocol(QSsl::TlsV1_2);
    // if(global_trust.enable_ca)
    //     ssl_config.setCaCertificates(QSslConfiguration::systemCaCertificates());
    // else
    //     ssl_config.setCaCertificates(QList<QSslCertificate> { });

    QNetworkRequest request(url);
    // request.setMaximumRedirectsAllowed(5);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, false);
    request.setSslConfiguration(ssl_config);

    this->current_reply = manager.get(request);
    if(this->current_reply == nullptr)
        return false;

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
        emit this->networkError(error, reply->errorString());
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

    qDebug() << "HTTP SSL Errors:";
    for(auto const & err : errors)
        qDebug() << err;
    this->current_reply->ignoreSslErrors();
}

void WebClient::on_redirected(const QUrl &url)
{
    qDebug() << "redirected to" << url;
}
