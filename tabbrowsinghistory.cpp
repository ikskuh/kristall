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

void TabBrowsingHistory::pushUrl(const QUrl &url)
{
    this->beginInsertRows(QModelIndex{}, this->history.length(),this->history.length() + 1);

    this->history.push_back(url);

    this->endInsertRows();
}

int TabBrowsingHistory::rowCount(const QModelIndex &parent) const
{
    return history.size();
}

bool TabBrowsingHistory::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}

QVariant TabBrowsingHistory::data(const QModelIndex &index, int role) const
{
    if(role != Qt::DisplayRole) {
        return QVariant{};
    }
    return history.at(index.row()).toString();
}

