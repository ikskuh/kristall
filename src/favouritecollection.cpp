#include "favouritecollection.hpp"
#include "ioutil.hpp"

#include <cassert>
#include <QDebug>
#include <QIcon>
#include <QMimeData>

#include <memory>

FavouriteCollection::FavouriteCollection(QObject *parent)
    : QAbstractItemModel(parent)
{

}

FavouriteCollection::FavouriteCollection(const FavouriteCollection &other)
{
    for(auto const & grp : other.root.children)
    {
        auto const & src_group = grp->as<GroupNode>();
        auto dst_group = std::make_unique<GroupNode>();

        dst_group->title = src_group.title;

        for(auto const & id : src_group.children) {
            auto const & src_id = id->as<FavouriteNode>();
            auto dst_id = std::make_unique<FavouriteNode>();

            dst_id->favourite = src_id.favourite;

            dst_group->children.emplace_back(std::move(dst_id));
        }

        root.children.emplace_back(std::move(dst_group));
    }


    relayout();
}

FavouriteCollection &FavouriteCollection::operator=(const FavouriteCollection & other)
{
    beginResetModel();

    root.children.clear();
    for(auto const & grp : other.root.children)
    {
        auto const & src_group = grp->as<GroupNode>();
        auto dst_group = std::make_unique<GroupNode>();

        dst_group->title = src_group.title;

        for(auto const & id : src_group.children) {
            auto const & src_id = id->as<FavouriteNode>();
            auto dst_id = std::make_unique<FavouriteNode>();

            dst_id->favourite = src_id.favourite;

            dst_group->children.emplace_back(std::move(dst_id));
        }

        root.children.emplace_back(std::move(dst_group));
    }

    this->relayout();
    endResetModel();
    return *this;
}

FavouriteCollection &FavouriteCollection::operator=(FavouriteCollection && other)
{
    beginResetModel();
    this->root.children = std::move(other.root.children);
    this->relayout();
    endResetModel();
    return *this;
}

void FavouriteCollection::load(QSettings &settings)
{
    this->beginResetModel();

    this->root.children.clear();

    int group_cnt = settings.beginReadArray("groups");
    for(int i = 0; i < group_cnt; i++)
    {
        settings.setArrayIndex(i);
        auto group = std::make_unique<GroupNode>();

        group->title = settings.value("name").toString();

        int id_cnt = settings.beginReadArray("favourites");

        for(int j = 0; j < id_cnt; j++)
        {
            settings.setArrayIndex(j);
            auto fav = std::make_unique<FavouriteNode>();

            fav->favourite.title = settings.value("title").toString();
            fav->favourite.destination = settings.value("url").toUrl();

            group->children.emplace_back(std::move(fav));
        }

        settings.endArray();

        this->root.children.emplace_back(std::move(group));
    }
    settings.endArray();

    relayout();

    this->endResetModel();
}

void FavouriteCollection::save(QSettings &settings) const
{
    settings.beginWriteArray("groups", int(root.children.size()));

    int grp_index = 0;
    for(auto const & grp : root.children)
    {
        settings.setArrayIndex(grp_index);
        grp_index += 1;

        auto & group = grp->as<GroupNode>();
        settings.setValue("name", group.title);

        settings.beginWriteArray("favourites", int(group.children.size()));

        int id_index = 0;
        for(auto const & _id : group.children)
        {
            settings.setArrayIndex(id_index);
            id_index += 1;

            auto & id = _id->as<FavouriteNode>();

            settings.setValue("title", id.favourite.title);
            settings.setValue("url", id.favourite.destination);
        }

        settings.endArray();
    }

    settings.endArray();
}

bool FavouriteCollection::addGroup(const QString &group_name)
{
    GroupNode * group;
    return internalAddGroup(group_name, group);
}

bool FavouriteCollection::addFavourite(const QString &group_name, const Favourite &fav)
{
    GroupNode * group;
    internalAddGroup(group_name, group);

    QModelIndex parent_index = createIndex(group->index, 0, group);

    beginInsertRows(parent_index, group->children.size(), group->children.size() + 1);

    auto id = std::make_unique<FavouriteNode>();
    id->favourite = fav;
    group->children.emplace_back(std::move(id));

    this->relayout();

    this->endInsertRows();

    return true;
}

void FavouriteCollection::editFavouriteTitle(const QModelIndex &index, const QString &title)
{
    this->getMutableFavourite(index)->title = title;
}

bool FavouriteCollection::editFavouriteTitle(const QUrl &u, const QString &new_title)
{
    QUrl url = IoUtil::uniformUrl(u);
    for(auto const & group : this->root.children)
    {
        for(auto const & ident : group->children)
        {
            FavouriteNode* node = &ident->as<FavouriteNode>();
            if(IoUtil::uniformUrl(node->favourite.destination) == url)
            {
                node->favourite.title = new_title;
                return true;
            }
        }
    }
    return false;
}

void FavouriteCollection::editFavouriteDest(const QModelIndex &index, const QUrl &url)
{
    this->getMutableFavourite(index)->destination = url;
}

Favourite FavouriteCollection::getFavourite(const QUrl &u) const
{
    QUrl url = IoUtil::uniformUrl(u);
    for(auto const & group : this->root.children)
    {
        for(auto const & ident : group->children)
        {
            FavouriteNode* node = &ident->as<FavouriteNode>();
            if(IoUtil::uniformUrl(node->favourite.destination) == url)
                return node->favourite;
        }
    }
    return Favourite();
}

Favourite FavouriteCollection::getFavourite(const QModelIndex &index) const
{
    if (!index.isValid())
        return Favourite();

    if (index.column() != 0)
        return Favourite();

    Node const *item = static_cast<Node const*>(index.internalPointer());
    switch(item->type) {
    case Node::Favourite: return static_cast<FavouriteNode const *>(item)->favourite;
    default:
        return Favourite();
    }
}

Favourite * FavouriteCollection::getMutableFavourite(const QModelIndex &index)
{
    if (!index.isValid())
        return nullptr;

    if (index.column() != 0)
        return nullptr;

    Node *item = static_cast<Node*>(index.internalPointer());
    switch(item->type) {
    case Node::Favourite: return &static_cast<FavouriteNode *>(item)->favourite;
    default:
        return nullptr;
    }
}

QStringList FavouriteCollection::groups() const
{
    QStringList result;
    for(auto const & grp : root.children)
    {
        result.append(grp->as<GroupNode>().title);
    }
    return result;
}

QString FavouriteCollection::group(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString { };

    Node const *item = static_cast<Node const*>(index.internalPointer());

    switch(item->type) {
    case Node::Root:     return QString { };
    case Node::Group:    return static_cast<GroupNode const *>(item)->title;
    case Node::Favourite: return static_cast<FavouriteNode const *>(item)->parent->as<GroupNode>().title;
    default:             return QString { };
    }
}

bool FavouriteCollection::destroyFavourite(const QModelIndex &index)
{
    if (!index.isValid())
        return false;

    Node * childItem = static_cast<Node *>(index.internalPointer());
    Node * parent = childItem->parent;

    if (parent == &root)
        return false;

    beginRemoveRows(this->parent(index), index.row(), index.row() + 1);

    parent->children.erase(parent->children.begin() + childItem->index);

    endRemoveRows();

    return true;
}

bool FavouriteCollection::canDeleteGroup(const QString &group_name)
{
    for(auto const & group_node : root.children)
    {
        auto & group = group_node->as<GroupNode>();
        if((group.children.size() == 0) and (group.title == group_name))
            return true;

    }
    return false;
}

bool FavouriteCollection::deleteGroup(const QString &group_name)
{
    size_t index = 0;
    for(auto it = root.children.begin(); it != root.children.end(); it++, index++)
    {
        auto & group = it->get()->as<GroupNode>();
        if(group.title == group_name) {
            if(group.children.size() > 0) {
                qDebug() << "cannot delete non-empty group" << group_name;
                return false;
            }

            beginRemoveRows(QModelIndex { }, index, index + 1);

            root.children.erase(it);

            endRemoveRows();

            return true;
        }
    }
    return false;
}

QVector<QPair<QString, Favourite const *>> FavouriteCollection::allFavourites() const
{
    QVector<QPair<QString, Favourite const *>> identities;

    for(auto const & group : this->root.children)
    {
        for(auto const & ident : group->children)
        {
            identities.append(QPair<QString, Favourite const *> {
                group->as<GroupNode>().title,
                &ident->as<FavouriteNode>().favourite
            });
        }
    }

    return identities;
}

bool FavouriteCollection::containsUrl(const QUrl &u) const
{
    QUrl url = IoUtil::uniformUrl(u);
    for(auto const & group : this->root.children)
    {
        for(auto const & ident : group->children)
        {
            if(IoUtil::uniformUrl(ident->as<FavouriteNode>().favourite.destination) == url)
                return true;
        }
    }
    return false;
}

bool FavouriteCollection::addUnsorted(const QUrl &url, const QString &t)
{
    if(containsUrl(url))
        return false;
    return addFavourite(tr("Unsorted"), Favourite {
        t,
        url,
    });
}

bool FavouriteCollection::removeUrl(const QUrl &u)
{
    QUrl url = IoUtil::uniformUrl(u);
    for(auto const & group : this->root.children)
    {
        size_t index = 0;
        for(auto it = group->children.begin(); it != group->children.end(); ++it, ++index)
        {
            auto & fav = it->get()->as<FavouriteNode>();
            if(IoUtil::uniformUrl(fav.favourite.destination) == url) {
                beginRemoveRows(QModelIndex { }, index, index + 1);

                group->children.erase(it);

                endRemoveRows();

                return true;
            }
        }
    }
    return false;
}

QModelIndex FavouriteCollection::index(int row, int column, const QModelIndex &parent) const
{
    if (not hasIndex(row, column, parent))
        return QModelIndex();

    Node const * parentItem;

    if(!parent.isValid())
        parentItem = &this->root;
    else
        parentItem = static_cast<Node*>(parent.internalPointer());

    auto & children = parentItem->children;
    if(row < 0 or size_t(row) >= children.size())
        return QModelIndex { };
    return createIndex(
        row,
        column,
        reinterpret_cast<quintptr>(children[row].get())
    );
}

QModelIndex FavouriteCollection::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    Node const  *childItem = static_cast<Node const *>(index.internalPointer());
    Node const * parent = childItem->parent;

    if (parent == &root)
        return QModelIndex();

    return createIndex(
        parent->index,
        0,
        reinterpret_cast<quintptr>(parent));
}

int FavouriteCollection::rowCount(const QModelIndex &parent) const
{
    Node const * parentItem;

    if (!parent.isValid())
        parentItem = &root;
    else
        parentItem = static_cast<Node const *>(parent.internalPointer());

    return parentItem->children.size();
}

int FavouriteCollection::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant FavouriteCollection::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Node const *item = static_cast<Node const*>(index.internalPointer());
    if (role == Qt::DisplayRole)
    {
        switch(item->type) {
        case Node::Root: return "root";
        case Node::Group: return static_cast<GroupNode const *>(item)->title;
        case Node::Favourite: return static_cast<FavouriteNode const *>(item)->favourite.getTitle();
        default:
            return "Unknown";
        }
    }
    if (role == Qt::EditRole)
    {
        switch(item->type) {
        case Node::Root: return "root";
        case Node::Group: return static_cast<GroupNode const *>(item)->title;
        case Node::Favourite: return static_cast<FavouriteNode const *>(item)->favourite.title;
        default:
            return "Unknown";
        }
    }
    else if(role == Qt::DecorationRole) {

        switch(item->type) {
        case Node::Root: return QVariant { };
        case Node::Group: return QIcon::fromTheme("folder");
        case Node::Favourite: return QIcon::fromTheme("favourite");
        default: return QVariant { };
        }
    }

    return QVariant();
}

bool FavouriteCollection::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    Node *item = static_cast<Node*>(index.internalPointer());
    if (role == Qt::EditRole)
    {
        switch(item->type) {
        case Node::Root: return false;
        case Node::Group:
            item->as<GroupNode>().title = value.toString();
            emit this->dataChanged(index, index, { Qt::EditRole });
            return true;
        case Node::Favourite:
            item->as<FavouriteNode>().favourite.title = value.toString();
            emit this->dataChanged(index, index, { Qt::EditRole });
            return true;
        default: return false;
        }
    }

    return false;
}

Qt::ItemFlags FavouriteCollection::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    Node const *item = static_cast<Node const*>(index.internalPointer());
    switch(item->type) {
    case Node::Favourite:
        return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
    case Node::Group:
        return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled | Qt::ItemIsEditable;
    default:
        return QAbstractItemModel::flags(index);
    }
}

QStringList FavouriteCollection::mimeTypes() const
{
    QStringList mimes;
    mimes << "x-kristall/identity";
    return mimes;
}

#include <QBuffer>
#include <QDataStream>

QMimeData *FavouriteCollection::mimeData(const QModelIndexList &indexes) const
{
    if(indexes.size() != 1)
        return nullptr;
    auto const & index = indexes.at(0);

    if (not index.isValid())
        return nullptr;

    Node const *item = static_cast<Node const*>(index.internalPointer());
    switch(item->type) {
    case Node::Favourite: {
        auto const & favourite = item->as<FavouriteNode>().favourite;

        QByteArray buffer;

        {
            QDataStream stream { &buffer, QIODevice::WriteOnly };

            stream << favourite.title;
            stream << favourite.destination;
        }
        assert(buffer.size() > 0);

        auto mime = std::make_unique<QMimeData>();

        mime->setData("x-kristall/favourite", buffer);

        return mime.release();
    }
    default:
        return nullptr;
    }
}

bool FavouriteCollection::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    if (not parent.isValid())
        return false;

    Node const *item = static_cast<Node const*>(parent.internalPointer());
    switch(item->type) {
    case Node::Group: {
        return data->hasFormat("x-kristall/favourite") and (action == Qt::MoveAction);
    }
    default:
        return false;
    }
}

bool FavouriteCollection::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(column);

    if(action != Qt::MoveAction)
        return false;

    if (not parent.isValid())
        return false;

    Node *item = static_cast<Node *>(parent.internalPointer());
    switch(item->type) {
    case Node::Group: {
        auto ident_blob = data->data("x-kristall/favourite");

        auto node = std::make_unique<FavouriteNode>();
        Favourite & fav = node->favourite;
        {
            QDataStream stream { &ident_blob, QIODevice::ReadOnly };

            stream >> fav.title;
            stream >> fav.destination;
        }

        if(not fav.isValid())
            return false;

        auto & insert_list = item->as<GroupNode>().children;

        if((row < 0) or (size_t(row) >= insert_list.size())) {
            beginInsertRows(parent, insert_list.size(), insert_list.size());
            insert_list.emplace_back(std::move(node));
        } else {
            beginInsertRows(parent, row, row);
            insert_list.emplace(insert_list.begin() + size_t(row), std::move(node));
        }
        endInsertRows();

        qDebug() << "dropping" << data->formats() << row;

        this->relayout();
        return true;
    }
    default:
        return false;
    }
}

Qt::DropActions FavouriteCollection::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions FavouriteCollection::supportedDragActions() const
{
    return Qt::MoveAction;
}

bool FavouriteCollection::removeRows(int row, int count, const QModelIndex &parent)
{
    if (not parent.isValid())
        return false;

    if(count != 1)
        return false;

    Node *item = static_cast<Node *>(parent.internalPointer());
    switch(item->type) {
    case Node::Group: {
        auto & children = item->as<GroupNode>().children;

        if((row < 0) or (size_t(row) >= children.size()))
            return false;

        beginRemoveRows(parent, row, row + 1);
        children.erase(children.begin() + size_t(row));
        endRemoveRows();

        return true;
    }
    default:
        return false;
    }
}

void FavouriteCollection::relayout()
{
    for(size_t i = 0; i < root.children.size(); i++)
    {
        auto & group = *root.children[i];
        group.parent = &root;
        group.index = i;

        // qDebug() << "group[" << group.index << "]" << group.as<GroupNode>().title;

        for(size_t j = 0; j < group.children.size(); j++)
        {
            auto & id = *group.children[j];
            id.parent = &group;
            id.index = j;
            assert(id.children.size() == 0);

            // qDebug() << "id[" << id.index << "]" << id.as<IdentityNode>().identity.display_name;
        }
    }
}

bool FavouriteCollection::internalAddGroup(const QString &group_name, GroupNode * & group)
{
    for(auto const & grp : root.children)
    {
        auto * g = static_cast<GroupNode*>(grp.get());
        if(g->title == group_name) {
            group = g;
            return false;
        }
    }

    auto parent = QModelIndex { };

    beginInsertRows(parent, this->root.children.size(), this->root.children.size() + 1);

    group = new GroupNode();
    group->title = group_name;
    this->root.children.emplace_back(group);

    this->relayout();

    endInsertRows();

    return true;
}
