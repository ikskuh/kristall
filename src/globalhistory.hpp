#ifndef GLOBALHISTORY_HPP
#define GLOBALHISTORY_HPP

#include <QAbstractListModel>
#include <QVector>
#include <QUrl>
#include <QString>
#include <QDateTime>

//! Session-wide list of visited urls, shared between all tabs and windows.
//! In-memory only, newest visit first.
class GlobalHistory : public QAbstractListModel
{
    Q_OBJECT

    struct Entry
    {
        QUrl url;
        QString title;
        QDateTime visit_time;
    };

public:
    explicit GlobalHistory(QObject * parent = nullptr);

    void addVisit(QUrl const & url);

    void setTitleForUrl(QUrl const & url, QString const & title);

    QUrl get(QModelIndex const & index) const;

    void clear();

public:
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

private:
    QVector<Entry> entries;
};

#endif // GLOBALHISTORY_HPP
