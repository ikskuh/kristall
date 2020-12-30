#include "filehandler.hpp"

#include "../kristall.hpp"

#include <QMimeDatabase>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QDir>

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
    else if (QDir dir = QDir(url.path()); dir.exists())
    {
        // URL points to directory - we create Gemtext
        // page which lists contents of directory.
        QString page;
        page += QString("# Index of %1\n").arg(url.path());

        auto filters = QDir::Dirs | QDir::Files | QDir::NoDot;
        if (kristall::options.show_hidden_files_in_dirs) filters |= QDir::Hidden;
        dir.setFilter(filters);

        // Iterate over files in the directory, and add links to each.
        for (unsigned i = 0; i < dir.count(); ++i)
        {
            // Add link to page.
            page += QString("=> file://%1 %2\n")
                .arg(QUrl(dir.filePath(dir[i])).toString(QUrl::FullyEncoded),
                dir[i]);
        }

        emit this->requestComplete(page.toUtf8(), "text/gemini");
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
