#include "cachehandler.hpp"
#include "kristall.hpp"

#include <QDebug>

void CacheHandler::push(const QUrl &url, const QByteArray &body, const MimeType &mime)
{
    QString urlstr = url.toString(QUrl::FullyEncoded | QUrl::RemoveFragment);

    if (this->page_cache.find(urlstr) != this->page_cache.end())
    {
        qDebug() << "Updating cached page";
        auto pg = this->page_cache[urlstr];
        pg->body = body;
        pg->mime = mime;
        return;
    }

    this->page_cache[urlstr] = std::make_shared<CachedPage>(url, body, mime);

    qDebug() << "Pushed page to cache: " << url;

    return;
}

std::shared_ptr<CachedPage> CacheHandler::find(const QString &url)
{
    if (this->page_cache.find(url) != this->page_cache.end())
    {
        return page_cache[url];
    }
    return nullptr;
}

std::shared_ptr<CachedPage> CacheHandler::find(const QUrl &url)
{
    return this->find(url.toString(QUrl::FullyEncoded | QUrl::RemoveFragment));
}

bool CacheHandler::contains(const QString &url) const
{
    return this->page_cache.find(url) != this->page_cache.end();
}

bool CacheHandler::contains(const QUrl &url) const
{
    return this->contains(url.toString(QUrl::FullyEncoded | QUrl::RemoveFragment));
}

CacheMap const& CacheHandler::getPages() const
{
    return this->page_cache;
}
