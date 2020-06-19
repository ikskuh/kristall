#include "trustedhostcollection.hpp"

#include <cassert>

TrustedHostCollection::TrustedHostCollection(QObject *parent)
    : QAbstractTableModel(parent)
{
}

TrustedHostCollection::TrustedHostCollection(const TrustedHostCollection & other) :
    items(other.items)
{
    assert(other.parent() == nullptr);

}

TrustedHostCollection::TrustedHostCollection(TrustedHostCollection &&other) :
    items(std::move(other.items))
{
    assert(other.parent() == nullptr);
}

QVariant TrustedHostCollection::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Vertical)
        return QVariant { };
    if(role == Qt::DisplayRole)
    {
        switch(section)
        {
        case 0: return "Host Name";
        case 1: return "First Seen";
        case 2: return "Key Type";
        }
    }
    return QVariant { };
}

int TrustedHostCollection::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return items.size();
}

int TrustedHostCollection::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 3;
}

QVariant TrustedHostCollection::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant { };
    if(index.row() < 0 or index.row() >= items.size())
        return QVariant { };

    auto const & host = items.at(index.row());

    if(role == Qt::DisplayRole)
    {
        switch(index.column())
        {
        case 0: return host.host_name;
        case 1: return host.trusted_at.toString();
        case 2: switch(host.public_key.algorithm())
            {
            case QSsl::Rsa: return "RSA";
            case QSsl::Ec: return "EC";
            // case QSsl::Dh: return "DH";
            case QSsl::Dsa: return "DSA";
            case QSsl::Opaque: return "Opaque";
            default: return "Unforseen";
            }
        }
    }
    return QVariant { };
}

TrustedHostCollection &TrustedHostCollection::operator=(const TrustedHostCollection & other)
{
    beginResetModel();
    this->items = other.items;
    endResetModel();
    return *this;
}

TrustedHostCollection &TrustedHostCollection::operator=(TrustedHostCollection && other)
{
    beginResetModel();
    this->items = std::move(other.items);
    endResetModel();
    return *this;
}

void TrustedHostCollection::clear()
{
    beginResetModel();
    this->items.clear();
    endResetModel();
}

bool TrustedHostCollection::insert(const TrustedHost &host)
{
    for(auto const & item : items)
    {
        if(item.host_name == host.host_name)
            return false;
    }

    beginInsertRows(QModelIndex { }, items.size(), items.size() + 1);
    items.append(host);
    endInsertRows();

    return true;
}

std::optional<TrustedHost> TrustedHostCollection::get(QString const & host_name) const
{
    for(auto const & item : items)
    {
        if(item.host_name == host_name)
            return item;
    }
    return std::nullopt;
}

std::optional<TrustedHost> TrustedHostCollection::get(const QModelIndex &index) const
{
    if(not index.isValid())
        return std::nullopt;
    if(index.row() < 0 or index.row() >= items.size())
        return std::nullopt;
    return items.at(index.row());
}

void TrustedHostCollection::remove(const QModelIndex &index)
{
    if(not index.isValid())
        return;
    if(index.row() < 0 or index.row() >= items.size())
        return;
    beginRemoveRows(QModelIndex{}, index.row(), index.row() + 1);
    items.removeAt(index.row());
    endRemoveRows();
}

QVector<TrustedHost> TrustedHostCollection::getAll() const
{
    return items;
}
