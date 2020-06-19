#include "filehandler.hpp"

#include <QMimeDatabase>
#include <QUrl>
#include <QFile>

FileHandler::FileHandler()
{

}

bool FileHandler::supportsScheme(const QString &scheme) const
{
    return (scheme == "file");
}

bool FileHandler::startRequest(const QUrl &url, RequestOptions options)
{
    Q_UNUSED(options)

    QFile file { url.path() };

    if (file.open(QFile::ReadOnly))
    {
        QMimeDatabase db;
        auto mime = db.mimeTypeForUrl(url).name();
        auto data = file.readAll();
        emit this->requestComplete(data, mime);
    }
    else
    {
        emit this->networkError(ResourceNotFound, "The requested file does not exist!");
    }
    return true;
}

bool FileHandler::isInProgress() const
{
    return false;
}

bool FileHandler::cancelRequest()
{
    return true;
}
