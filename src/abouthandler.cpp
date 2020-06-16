#include "abouthandler.hpp"
#include "kristall.hpp"

#include <QUrl>
#include <QFile>

AboutHandler::AboutHandler()
{

}

bool AboutHandler::supportsScheme(const QString &scheme) const
{
    return (scheme == "about");
}

bool AboutHandler::startRequest(const QUrl &url)
{
    if (url.path() == "blank")
    {
        emit this->requestComplete("", "text/gemini");
    }
    else if (url.path() == "favourites")
    {
        QByteArray document;

        document.append("# Favourites\n");
        document.append("\n");

        for (auto const &fav : global_favourites.getAll())
        {
            document.append("=> " + fav.toString().toUtf8() + "\n");
        }

        this->requestComplete(document, "text/gemini");
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
