#include "documentoutlinemodel.hpp"

#include <QModelIndex>

DocumentOutlineModel::DocumentOutlineModel() :
    QAbstractItemModel(),
    root()
{

}

void DocumentOutlineModel::clear()
{
    beginBuild();
    endBuild();

}

void DocumentOutlineModel::beginBuild()
{
    beginResetModel();
    root = Node {
        nullptr,
        "<ROOT>", "",
        0, 0,
        QList<Node> { },
    };
}

void DocumentOutlineModel::appendH1(const QString &title, QString const & anchor)
{
    root.children.append(Node {
        &root,
        title, anchor,
        1, 0,
        QList<Node> { },
    });
}

void DocumentOutlineModel::appendH2(const QString &title, QString const & anchor)
{
    auto & parent = ensureLevel1();
    parent.children.append(Node {
        &parent,
        title, anchor,
        2, parent.children.size() - 1,
        QList<Node> { },
    });
}

void DocumentOutlineModel::appendH3(const QString &title, QString const & anchor)
{
    auto & parent = ensureLevel2();
    parent.children.append(Node {
        &parent,
        title, anchor,
        3, parent.children.size() - 1,
        QList<Node> { },
    });
}

void DocumentOutlineModel::endBuild()
{
    for(auto const & h1 : this->root.children)
    {
        assert(h1.depth == 1);
        assert(h1.parent == &this->root);
        for(auto const & h2 : h1.children)
        {
            assert(h2.depth == 2);
            assert(h2.parent == &h1);
            for(auto const & h3 : h2.children)
            {
                assert(h3.depth == 3);
                assert(h3.parent == &h2);
            }
        }
    }
    endResetModel();
}

QString DocumentOutlineModel::getTitle(const QModelIndex &index) const
{
    if(not index.isValid())
        return "";

    Node const  *childItem = static_cast<Node const *>(index.internalPointer());

    return childItem->title;
}

QString DocumentOutlineModel::getAnchor(const QModelIndex &index) const
{
    if(not index.isValid())
        return "";

    Node const  *childItem = static_cast<Node const *>(index.internalPointer());

    return childItem->anchor;
}

QModelIndex DocumentOutlineModel::index(int row, int column, const QModelIndex &parent) const
{
    if (not hasIndex(row, column, parent))
        return QModelIndex();

    Node const * parentItem;

    if (!parent.isValid())
        parentItem = &this->root;
    else
        parentItem = static_cast<Node*>(parent.internalPointer());

    Node const * childItem = &parentItem->children[row];
    if (childItem)
        return createIndex(row, column, reinterpret_cast<quintptr>(childItem));
    return QModelIndex();

}

QModelIndex DocumentOutlineModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    Node const  *childItem = static_cast<Node const *>(child.internalPointer());
    Node const * parent = childItem->parent;

    if (parent == &root)
        return QModelIndex();

    return createIndex(
        parent->index,
        0,
        reinterpret_cast<quintptr>(parent));
}

int DocumentOutlineModel::rowCount(const QModelIndex &parent) const
{
    Node const *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = &root;
    else
        parentItem = static_cast<Node const *>(parent.internalPointer());

    return parentItem->children.size();
}

int DocumentOutlineModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant DocumentOutlineModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.column() != 0)
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    Node const *item = static_cast<Node const*>(index.internalPointer());

    return item->title;
}

DocumentOutlineModel::Node & DocumentOutlineModel::ensureLevel1()
{
    if(root.children.size() == 0) {
        root.children.append(Node {
            &root,
            "<missing layer>", "",
            1, 0,
            QList<Node> { },
        });
    }
    return root.children.last();
}

DocumentOutlineModel::Node & DocumentOutlineModel::ensureLevel2()
{
    auto & parent = ensureLevel1();

    if(parent.children.size() == 0) {
        root.children.append(Node {
            &parent,
            "<missing layer>", "",
            2, 0,
            QList<Node> { },
        });
    }

    return parent.children.last();
}
