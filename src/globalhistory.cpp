#include "globalhistory.hpp"

GlobalHistory::GlobalHistory(QObject * parent)
    : QAbstractListModel(parent)
{

}

void GlobalHistory::addVisit(const QUrl &url)
{
    if(not url.isValid())
        return;

    if(not this->entries.isEmpty() and this->entries.first().url == url)
        return;

    this->beginInsertRows(QModelIndex{}, 0, 0);
    this->entries.prepend(Entry {
        url,
        QString { },
        QDateTime::currentDateTime(),
    });
    this->endInsertRows();
}

void GlobalHistory::setTitleForUrl(const QUrl &url, const QString &title)
{
    for(int i = 0; i < this->entries.size(); i++)
    {
        if(this->entries.at(i).url == url) {
            this->entries[i].title = title;
            emit this->dataChanged(this->index(i), this->index(i));
            return;
        }
    }
}

QUrl GlobalHistory::get(const QModelIndex &index) const
{
    if(not index.isValid())
        return QUrl { };

    if(index.row() >= this->entries.size())
        return QUrl { };

    return this->entries.at(index.row()).url;
}

void GlobalHistory::clear()
{
    this->beginResetModel();
    this->entries.clear();
    this->endResetModel();
}

int GlobalHistory::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return this->entries.size();
}

QVariant GlobalHistory::data(const QModelIndex &index, int role) const
{
    if(not index.isValid() or index.row() >= this->entries.size())
        return QVariant { };

    auto const & entry = this->entries.at(index.row());

    switch(role)
    {
    case Qt::DisplayRole:
        return entry.url.toString();

    case Qt::ToolTipRole:
        if(entry.title.isEmpty())
            return entry.visit_time.toString(Qt::TextDate);
        else
            return QString("%1\n%2").arg(entry.title, entry.visit_time.toString(Qt::TextDate));

    default:
        return QVariant { };
    }
}
