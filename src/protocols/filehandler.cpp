#include "filehandler.hpp"

#include <QMimeDatabase>
#include <QUrl>
#include <QFile>
#include <QFileInfo>

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
        auto data = file.readAll();
        QString mime;

        // Find mime type of file. We detect text/gemini
        // using the file suffix.
        QString suffix = QFileInfo(file).completeSuffix();
        if (suffix == "gmi")
        {
            mime = "text/gemini";
        }
        else
        {
            QMimeDatabase db;
            mime = db.mimeTypeForUrl(url).name();
        }

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
