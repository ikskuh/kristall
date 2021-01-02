#ifndef FAVOURITECOLLECTION_HPP
#define FAVOURITECOLLECTION_HPP

#include <QAbstractItemModel>
#include <QUrl>
#include <QString>
#include <memory>
#include <QSettings>

struct Favourite
{
    QString title;
    QUrl destination;

    bool isValid() const {
        return destination.isValid();
    }

    QString getTitle() const {
        if(title.isEmpty())
            return destination.toString(QUrl::FullyEncoded);
        else
            return title;
    }
};

class FavouriteCollection : public QAbstractItemModel
{
    Q_OBJECT
    struct Node  {
        enum Type { Root, Group, Favourite };
        Node * parent = nullptr;
        int index = 0;
        std::vector<std::unique_ptr<Node>> children;
        Type type;
        explicit Node(Type t) : type(t) { }
        virtual ~Node() = default;

        template<typename T>
        T & as() { return *static_cast<T*>(this); }

        template<typename T>
        T const & as() const { return *static_cast<T const*>(this); }
    };

    struct FavouriteNode : Node {
        ::Favourite favourite;
        FavouriteNode() : Node(Favourite) { }
        ~FavouriteNode() override = default;
    };

    struct GroupNode : Node {
        QString title;
        GroupNode() : Node(Group) { }
        ~GroupNode() override = default;
    };

    struct RootNode : Node {
        RootNode() : Node(Root) { }
        ~RootNode() override = default;
    };

public:
    explicit FavouriteCollection(QObject *parent = nullptr);

    FavouriteCollection(FavouriteCollection const & other);

    FavouriteCollection & operator=(FavouriteCollection const &);
    FavouriteCollection & operator=(FavouriteCollection &&);

public:
    void load(QSettings & settings);

    void save(QSettings & settings) const;

    bool addGroup(QString const & group);

    bool addFavourite(QString const & group, Favourite const & fav);

    void editFavouriteTitle(const QModelIndex &index, const QString &title);

    void editFavouriteDest(const QModelIndex & index, const QUrl & url);

    Favourite getFavourite(QModelIndex const & index) const;

    Favourite * getMutableFavourite(QModelIndex const & index);

    QStringList groups() const;

    //! Returns the group name of the index.
    QString group(QModelIndex const & index) const;

    bool destroyFavourite(QModelIndex const & index);

    bool canDeleteGroup(QString const & group_name);
    bool deleteGroup(QString const & group_name);

    //! Returns a list of non-mutable references to all contained favourites.
    //! Note that the group will change in-order, so all favourites for group a
    //! will be listed, then all for group b, no intermixing is done.
    QVector<QPair<QString, Favourite const *>> allFavourites() const;

    bool containsUrl(QUrl const & url) const;

    bool addUnsorted(QUrl const & url, QString const & title);

    bool removeUrl(QUrl const & url);

public:
    // Header:
    // QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    // Drag'n'Drop

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    Qt::DropActions supportedDropActions() const override;

    Qt::DropActions supportedDragActions() const override;

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;


private:
    void relayout();

    bool internalAddGroup(QString const & group_name, GroupNode * & out_group);

private:
    RootNode root;
};

#endif // FAVOURITECOLLECTION_HPP
