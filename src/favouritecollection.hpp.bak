#ifndef FAVOURITECOLLECTION_HPP
#define FAVOURITECOLLECTION_HPP

#include <QObject>
#include <QAbstractListModel>
#include <QUrl>
#include <QSettings>


class FavouriteCollection : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit FavouriteCollection(QObject *parent = nullptr);

    void add(QUrl const & url);

    void remove(QUrl const & url);

    bool contains(QUrl const & url);

    QUrl get(QModelIndex const & index) const ;

    bool save(QString const & fileName) const;
    bool save(QSettings & settings) const;

    bool load(QString const & fileName);
    bool load(QSettings & settings);

    QVector<QUrl> getAll() const {
        return this->items;
    }

public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

signals:

private:
    QVector<QUrl> items;

};

#endif // FAVOURITECOLLECTION_HPP
