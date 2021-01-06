#ifndef CACHEHANDLER_HPP
#define CACHEHANDLER_HPP

#include "mimeparser.hpp"
#include <memory>
#include <unordered_map>
#include <QUrl>
#include <QByteArray>

struct CachedPage
{
    QUrl url;

    QByteArray body;

    MimeType mime;

    int scroll_pos;

    // also: maybe compress page contents? May test
    // to see if it's worth it

    CachedPage(const QUrl &url, const QByteArray &body, const MimeType &mime)
        : url(url), body(body), mime(mime), scroll_pos(-1)
    {}
};

typedef std::unordered_map<QString, std::shared_ptr<CachedPage>> CacheMap;

class CacheHandler
{
public:
    void push(QUrl const & url, QByteArray const & body, MimeType const & mime);

    std::shared_ptr<CachedPage> find(QString const &url);
    std::shared_ptr<CachedPage> find(QUrl const &url);

    bool contains(QString const & url) const;
    bool contains(QUrl const & url) const;

    CacheMap const& getPages() const;

private:
    // In-memory cache storage.
    CacheMap page_cache;
};

#endif
