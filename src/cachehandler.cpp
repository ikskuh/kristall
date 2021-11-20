#include "cachehandler.hpp"
#include "kristall.hpp"
#include "ioutil.hpp"

#include <QDebug>

QString CacheHandler::cleanUrl(QUrl const & url)
{
    return url.toString(QUrl::FullyEncoded | QUrl::RemoveFragment);
}

void CacheHandler::push(const QUrl &url, const QByteArray &body, const MimeType &mime)
{
    // Skip if this item is above the cached item size threshold
    int bodysize = body.size();
    if (bodysize > (kristall::globals().options.cache_threshold * 1024))
    {
        qDebug() << "cache: item exceeds threshold (" << IoUtil::size_human(body.size()) << ")";
        return;
    }

    // Pop cached items until we are below the cache limit
    while ((bodysize + this->size()) > (kristall::globals().options.cache_limit * 1024))
    {
        this->popOldest();
    }

    QString urlstr = cleanUrl(url);

    auto it = this->page_cache.find(urlstr);
    if (it != this->page_cache.end())
    {
        qDebug() << "cache: updating page" << urlstr;
        auto pg = it->second;
        pg->body = body;
        pg->mime = mime;
        pg->time_cached = QDateTime::currentDateTime();
    }
    else
    {
        this->page_cache.emplace(urlstr, std::make_shared<CachedPage>(url, body, mime, QDateTime::currentDateTime()));
        qDebug() << "cache: pushing url " << urlstr;
    }
}


std::shared_ptr<CachedPage> CacheHandler::find(const QUrl &url)
{
    QString urltext = cleanUrl(url);

    auto it = this->page_cache.find(urltext);
    if(it != this->page_cache.end())
    {
        return it->second;
    }
    return nullptr;
}

bool CacheHandler::contains(const QUrl &url)
{
    return this->page_cache.find(cleanUrl(url)) != this->page_cache.end();
}

int CacheHandler::size()
{
    int s = 0;

    for (auto& i : this->page_cache)
        s += i.second->body.size();

    return s;
}

// Clears expired pages out of cache
void CacheHandler::clean()
{
    // Don't clean anything if we have unlimited item life.
    if (kristall::globals().options.cache_unlimited_life) return;

    // Find list of keys to delete
    std::vector<QString> vec;
    for (auto&& i : this->page_cache)
    {
        // Check if this cache item is expired.
        if (QDateTime::currentDateTime() > i.second->time_cached
            .addSecs(kristall::globals().options.cache_life * 60))
        {
            vec.emplace_back(std::move(i.first));
        }
    }

    // Delete them
    int count = 0;
    for (auto&& key : vec)
    {
        this->page_cache.erase(key);
        ++count;
    }

    if (count) qDebug() << "cache: cleaned " << count << " expired pages out of cache";
}

CacheMap const& CacheHandler::getPages() const
{
    return this->page_cache;
}

void CacheHandler::popOldest()
{
    if (this->page_cache.size() == 0)
    {
        return;
    }

    // This will iterate over the cache map,
    // find the key with the oldest timestamp,
    // and erase it from the map.
    //
    // (TODO: make this more efficient somehow?)

    QDateTime oldest = QDateTime::currentDateTime();
    QString oldest_key;
    for (auto it = this->page_cache.begin(); it != this->page_cache.end(); ++it)
    {
        if (it->second->time_cached < oldest)
        {
            oldest = it->second->time_cached;
            oldest_key = it->first;
        }
    }

    // Erase it from the map
    qDebug() << "cache: popping " << oldest_key;
    this->page_cache.erase(oldest_key);
}
