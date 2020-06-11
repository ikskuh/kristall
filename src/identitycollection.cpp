#include "identitycollection.hpp"

#include <cassert>
#include <QDebug>

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
            settings.setValue("certificate", id.identity.certificate.toDer());
            settings.setValue("private_key", id.identity.private_key.toDer());
        }


        settings.endArray();
    }

    settings.endArray();
}

bool IdentityCollection::addCertificate(const QString &group_name, const CryptoIdentity &crypto_id)
{
    // Don't allow saving transient certificates
    if(not crypto_id.is_persistent)
        return false;

    this->beginResetModel();

    GroupNode * group = nullptr;
    for(auto const & grp : root.children)
    {
        auto * g = static_cast<GroupNode*>(grp.get());
        if(g->title == group_name) {
            group = g;
            break;
        }
    }
    if(group == nullptr) {
        group = new GroupNode();
        group->title = group_name;
        this->root.children.emplace_back(group);
    }

    auto id = std::make_unique<IdentityNode>();
    id->identity = crypto_id;
    group->children.emplace_back(std::move(id));

    this->relayout();

    this->endResetModel();

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

QStringList IdentityCollection::groups() const
{
    QStringList result;
    for(auto const & grp : root.children)
    {
        result.append(grp->as<GroupNode>().title);
    }
    return result;
}

//QVariant IdentityCollection::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    return QVariant { };
//}

QModelIndex IdentityCollection::index(int row, int column, const QModelIndex &parent) const
{
    qDebug() << "index" << row << column << parent;
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
    qDebug() << "parent" << index;
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

    int rc = parentItem->children.size();
    qDebug() << "row count for " << parent << rc;
    return rc;
}

int IdentityCollection::columnCount(const QModelIndex &parent) const
{
    qDebug() << "column count" << parent;
    return 1;
}

QVariant IdentityCollection::data(const QModelIndex &index, int role) const
{
    qDebug() << "data" << index << role;
    if (!index.isValid())
        return QVariant();

    if (index.column() != 0)
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    Node const *item = static_cast<Node const*>(index.internalPointer());
    switch(item->type) {
    case Node::Root: return "root";
    case Node::Group: return static_cast<GroupNode const *>(item)->title;
    case Node::Identity: return static_cast<IdentityNode const *>(item)->identity.display_name;
    default:
        return "Unknown";
    }
}

void IdentityCollection::relayout()
{
    for(size_t i = 0; i < root.children.size(); i++)
    {
        auto & group = *root.children[i];
        group.parent = &root;
        group.index = i;

        qDebug() << "group[" << group.index << "]" << group.as<GroupNode>().title;

        for(size_t j = 0; j < group.children.size(); j++)
        {
            auto & id = *group.children[j];
            id.parent = &group;
            id.index = j;
            assert(id.children.size() == 0);

            qDebug() << "id[" << id.index << "]" << id.as<IdentityNode>().identity.display_name;
        }
    }
}
