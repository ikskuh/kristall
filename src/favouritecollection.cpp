#include "favouritecollection.hpp"

#include <QFile>

FavouriteCollection::FavouriteCollection(QObject *parent) :
    QAbstractListModel(parent)
{

}

void FavouriteCollection::add(QUrl const & url)
{
    if(contains(url))
        return;

    beginInsertRows(QModelIndex{}, items.size(), items.size() + 1);
    items.push_back(url);
    endInsertRows();
}

void FavouriteCollection::remove(QUrl const & url)
{
    for(int i = 0; i < items.size(); i++)
    {
        if(items.at(i) == url) {
            beginRemoveRows(QModelIndex{}, i, i + 1);
            items.removeAt(i);
            endRemoveRows();
            return;
        }
    }
}

bool FavouriteCollection::contains(const QUrl &url)
{
    for(auto const & item : items) {
        if(item == url)
            return true;
    }
    return false;
}

QUrl FavouriteCollection::get(const QModelIndex &index) const
{
    if(index.isValid()) {
        return items.at(index.row());
    } else {
        return QUrl { };
    }
}

bool FavouriteCollection::save(const QString &fileName) const
{
    QFile file(fileName);
    if(not file.open(QFile::WriteOnly))
        return false;

    for(auto const & url: items)
    {
        QByteArray blob = (url.toString() + "\n").toUtf8();

        qint64 offset = 0;
        while(offset < blob.size())
        {
            auto len = file.write(blob.data() + offset, blob.size() - offset);
            if(len <= 0) {
                file.close();
                return false;
            }
            offset += len;
        }
    }

    file.close();
    return true;
}

bool FavouriteCollection::save(QSettings &settings) const
{
    settings.beginWriteArray("favourites", items.size());
    for(int i = 0; i < items.size(); i++)
    {
        settings.setArrayIndex(i);
        settings.setValue("url", items[i].toString());
    }
    settings.endArray();
    return true;
}

bool FavouriteCollection::load(const QString &fileName)
{
    QFile file(fileName);
    if(not file.open(QFile::ReadOnly))
        return false;
    auto data = file.readAll();

    beginResetModel();

    items.clear();
    for(auto line : data.split('\n')) {
        if(line.size() > 0) {
            items.push_back(QUrl(QString::fromUtf8(line)));
        }
    }
    endResetModel();

    return true;
}

bool FavouriteCollection::load(QSettings & settings)
{
    int len = settings.beginReadArray("favourites");
    items.resize(len);
    for(int i = 0; i < items.size(); i++)
    {
        settings.setArrayIndex(i);
        items[i] = settings.value("url").toString();
    }
    settings.endArray();
    return true;
}

int FavouriteCollection::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return items.size();
}

bool FavouriteCollection::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(value)
    Q_UNUSED(index)
    Q_UNUSED(role)
    return false;
}

QVariant FavouriteCollection::data(const QModelIndex &index, int role) const
{
    if(role != Qt::DisplayRole) {
        return QVariant{};
    }
    return items.at(index.row()).toString();
}

