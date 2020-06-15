#include "identitycollection.hpp"

#include <cassert>
#include <QDebug>
#include <QIcon>

IdentityCollection::IdentityCollection(QObject *parent)
    : QAbstractItemModel(parent)
{

}

void IdentityCollection::load(QSettings &settings)
{
    this->beginResetModel();

    this->root.children.clear();

    int group_cnt = settings.beginReadArray("groups");
    for(int i = 0; i < group_cnt; i++)
    {
        settings.setArrayIndex(i);
        auto group = std::make_unique<GroupNode>();

        group->title = settings.value("name").toString();

        int id_cnt = settings.beginReadArray("identities");

        for(int j = 0; j < id_cnt; j++)
        {
            settings.setArrayIndex(j);
            auto id = std::make_unique<IdentityNode>();

            id->identity.is_persistent = true;
            id->identity.display_name = settings.value("display_name").toString();
            id->identity.user_notes = settings.value("user_notes").toString();

            id->identity.certificate = QSslCertificate::fromData(
                settings.value("certificate").toByteArray(),
                QSsl::Der
            ).first();

            id->identity.private_key = QSslKey(
                settings.value("private_key").toByteArray(),
                QSsl::Rsa,
                QSsl::Der
            );

            group->children.emplace_back(std::move(id));
        }

        settings.endArray();

        this->root.children.emplace_back(std::move(group));
    }
    settings.endArray();

    relayout();

    this->endResetModel();
}

void IdentityCollection::save(QSettings &settings) const
{
    settings.beginWriteArray("groups", int(root.children.size()));

    int grp_index = 0;
    for(auto const & grp : root.children)
    {
        settings.setArrayIndex(grp_index);
        grp_index += 1;

        auto & group = grp->as<GroupNode>();
        settings.setValue("name", group.title);

        settings.beginWriteArray("identities", int(group.children.size()));

        int id_index = 0;
        for(auto const & _id : group.children)
        {
            settings.setArrayIndex(id_index);
            id_index += 1;

            auto & id = _id->as<IdentityNode>();

            settings.setValue("display_name",  id.identity.display_name);
            settings.setValue("user_notes",  id.identity.user_notes);
            settings.setValue("certificate", id.identity.certificate.toDer());
            settings.setValue("private_key", id.identity.private_key.toDer());
        }


        settings.endArray();
    }

    settings.endArray();
}

bool IdentityCollection::addGroup(const QString &group_name)
{
    GroupNode * group;
    return internalAddGroup(group_name, group);
}

bool IdentityCollection::addCertificate(const QString &group_name, const CryptoIdentity &crypto_id)
{
    // Don't allow saving transient certificates
    if(not crypto_id.is_persistent)
        return false;

    GroupNode * group;
    internalAddGroup(group_name, group);

    QModelIndex parent_index = createIndex(group->index, 0, group);

    beginInsertRows(parent_index, group->children.size(), group->children.size() + 1);

    auto id = std::make_unique<IdentityNode>();
    id->identity = crypto_id;
    group->children.emplace_back(std::move(id));

    this->relayout();

    this->endInsertRows();

    return true;
}

CryptoIdentity IdentityCollection::getIdentity(const QModelIndex &index) const
{
    if (!index.isValid())
        return CryptoIdentity();

    if (index.column() != 0)
        return CryptoIdentity();

    Node const *item = static_cast<Node const*>(index.internalPointer());
    switch(item->type) {
    case Node::Identity: return static_cast<IdentityNode const *>(item)->identity;
    default:
        return CryptoIdentity();
    }
}

CryptoIdentity * IdentityCollection::getMutableIdentity(const QModelIndex &index)
{
    if (!index.isValid())
        return nullptr;

    if (index.column() != 0)
        return nullptr;

    Node *item = static_cast<Node*>(index.internalPointer());
    switch(item->type) {
    case Node::Identity: return &static_cast<IdentityNode *>(item)->identity;
    default:
        return nullptr;
    }
}

QStringList IdentityCollection::groups() const
{
    QStringList result;
    for(auto const & grp : root.children)
    {
        result.append(grp->as<GroupNode>().title);
    }
    return result;
}

QString IdentityCollection::group(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString { };

    Node const *item = static_cast<Node const*>(index.internalPointer());

    switch(item->type) {
    case Node::Root:     return QString { };
    case Node::Group:    return static_cast<GroupNode const *>(item)->title;
    case Node::Identity: return static_cast<IdentityNode const *>(item)->parent->as<GroupNode>().title;
    default:             return QString { };
    }
}

bool IdentityCollection::destroyIdentity(const QModelIndex &index)
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

bool IdentityCollection::canDeleteGroup(const QString &group_name)
{
    for(auto const & group_node : root.children)
    {
        auto & group = group_node->as<GroupNode>();
        if((group.children.size() == 0) and (group.title == group_name))
            return true;

    }
    return false;
}

bool IdentityCollection::deleteGroup(const QString &group_name)
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

QModelIndex IdentityCollection::index(int row, int column, const QModelIndex &parent) const
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

QModelIndex IdentityCollection::parent(const QModelIndex &index) const
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

int IdentityCollection::rowCount(const QModelIndex &parent) const
{
    Node const * parentItem;

    if (!parent.isValid())
        parentItem = &root;
    else
        parentItem = static_cast<Node const *>(parent.internalPointer());

    return parentItem->children.size();
}

int IdentityCollection::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant IdentityCollection::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Node const *item = static_cast<Node const*>(index.internalPointer());
    if (role == Qt::DisplayRole)
    {
        switch(item->type) {
        case Node::Root: return "root";
        case Node::Group: return static_cast<GroupNode const *>(item)->title;
        case Node::Identity: return static_cast<IdentityNode const *>(item)->identity.display_name;
        default:
            return "Unknown";
        }
    }
    else if(role == Qt::DecorationRole) {

        switch(item->type) {
        case Node::Root: return QVariant { };
        case Node::Group: return QIcon(":/icons/folder-open.svg");
        case Node::Identity: return QIcon(":/icons/certificate.svg");
        default: return QVariant { };
        }
    }

    return QVariant();
}

void IdentityCollection::relayout()
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

bool IdentityCollection::internalAddGroup(const QString &group_name, GroupNode * & group)
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
