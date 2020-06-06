#include "webclient.hpp"

#include <QNetworkRequest>
#include <QNetworkReply>

WebClient::WebClient(QObject *parent) :
    QObject(parent),
    current_reply(nullptr)
{
    manager.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

WebClient::~WebClient()
{

}

bool WebClient::startRequest(const QUrl &url)
{
    if(this->current_reply != nullptr)
        return true;

    this->body.clear();

    QNetworkRequest request(url);
    request.setMaximumRedirectsAllowed(5);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    this->current_reply = manager.get(request);
    if(this->current_reply == nullptr)
        return false;

    connect(this->current_reply, &QNetworkReply::readyRead, this, &WebClient::on_data);
    connect(this->current_reply, &QNetworkReply::finished, this,  &WebClient::on_finished);

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
}

void WebClient::on_finished()
{
    if(this->current_reply->error() != QNetworkReply::NoError)
    {
        emit this->requestFailed(this->current_reply->errorString());
    }
    else
    {
        auto mime = this->current_reply->header(QNetworkRequest::ContentTypeHeader).toString();

        qDebug() << this->current_reply->url() << mime;

        emit this->requestComplete(this->body, mime);

        this->body.clear();
    }
    this->current_reply->deleteLater();
    this->current_reply = nullptr;
}
