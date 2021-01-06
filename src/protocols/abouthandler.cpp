#include "abouthandler.hpp"
#include "kristall.hpp"
#include "ioutil.hpp"

#include <QUrl>
#include <QFile>

AboutHandler::AboutHandler()
{

}

bool AboutHandler::supportsScheme(const QString &scheme) const
{
    return (scheme == "about");
}

bool AboutHandler::startRequest(const QUrl &url, ProtocolHandler::RequestOptions options)
{
    Q_UNUSED(options)
    if (url.path() == "blank")
    {
        emit this->requestComplete("", "text/gemini");
    }
    else if (url.path() == "favourites")
    {
        QByteArray document;

        document.append("# Favourites\n");

        QString current_group;

        for (auto const &fav : kristall::favourites.allFavourites())
        {
            if(current_group != fav.first) {

                document.append("\n");
                document.append(QString("## %1\n").arg(fav.first).toUtf8());

                current_group = fav.first;
            }

            if(fav.second->title.isEmpty()) {
                document.append("=> " + fav.second->destination.toString().toUtf8() + "\n");
            } else {
                document.append("=> " + fav.second->destination.toString().toUtf8() + " " + fav.second->title.toUtf8() + "\n");
            }
        }

        emit this->requestComplete(document, "text/gemini");
    }
    else if (url.path() == "cache")
    {
        QByteArray document;
        document.append("# Cache information\n");

        auto& cache = kristall::cache.getPages();
        long unsigned cache_usage = 0;
        int cached_count = 0;
        for (auto it = cache.begin(); it != cache.end(); ++it, ++cached_count)
        {
            cache_usage += (long unsigned)it->second->body.size();
        }

        document.append(QString(
            "In-memory cache usage:\n"
            "* %1 used\n"
            "* %2 pages in cache\n")
            .arg(IoUtil::size_human(cache_usage), QString::number(cached_count)));

        emit this->requestComplete(document, "text/gemini");
    }
    else
    {
        QFile file(QString(":/about/%1.gemini").arg(url.path()));
        if (file.open(QFile::ReadOnly))
        {
            emit this->requestComplete(file.readAll(), "text/gemini");
        }
        else
        {
            emit this->networkError(ResourceNotFound, "The requested resource does not exist.");
        }
    }
    return true;
}

bool AboutHandler::isInProgress() const
{
    return false;
}

bool AboutHandler::cancelRequest()
{
    return true;
}
