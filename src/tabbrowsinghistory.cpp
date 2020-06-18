#include "tabbrowsinghistory.hpp"

TabBrowsingHistory::TabBrowsingHistory()
{

}

bool TabBrowsingHistory::canGoBack() const
{
    return this->history.size() > 0;
}

bool TabBrowsingHistory::canGoForward() const
{
    return false;
}

QModelIndex TabBrowsingHistory::pushUrl(QModelIndex const & position, const QUrl &url)
{
    this->beginInsertRows(QModelIndex{}, this->history.length(),this->history.length() + 1);

    if(position.isValid()) {
        this->history.resize(position.row() + 1);
    }

    this->history.push_back(url);

    this->endInsertRows();

    return this->createIndex(this->history.size() - 1, 0);
}

QUrl TabBrowsingHistory::get(const QModelIndex &index) const
{
    if(not index.isValid())
        return QUrl { };

    if(index.row() >= history.size())
        return QUrl { };
    else
        return history.at(index.row());
}

QModelIndex TabBrowsingHistory::oneForward(QModelIndex index) const
{
    if(not index.isValid())
        return QModelIndex{};
    if(index.row() >= history.size() - 1)
        return QModelIndex{};
    return createIndex(index.row() + 1, index.column());
}

QModelIndex TabBrowsingHistory::oneBackward(QModelIndex index) const
{
    if(not index.isValid())
        return QModelIndex{};
    if(index.row() == 0)
        return QModelIndex{};
    return createIndex(index.row() - 1, index.column());
}

int TabBrowsingHistory::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return history.size();
}

bool TabBrowsingHistory::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(index)
    Q_UNUSED(value)
    Q_UNUSED(role)
    return false;
}

QVariant TabBrowsingHistory::data(const QModelIndex &index, int role) const
{
    if(role != Qt::DisplayRole) {
        return QVariant{};
    }
    return history.at(index.row()).toString();
}

