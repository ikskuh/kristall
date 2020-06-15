#ifndef TRUSTEDHOSTCOLLECTION_HPP
#define TRUSTEDHOSTCOLLECTION_HPP

#include <QAbstractTableModel>

#include "trustedhost.hpp"
#include <optional>

class TrustedHostCollection : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TrustedHostCollection(QObject *parent = nullptr);

    TrustedHostCollection(TrustedHostCollection const &);
    TrustedHostCollection(TrustedHostCollection &&);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public:
    TrustedHostCollection & operator=(TrustedHostCollection const &);
    TrustedHostCollection & operator=(TrustedHostCollection &&);

    void clear();

    bool insert(TrustedHost const & host);

    std::optional<TrustedHost> get(QString const & host_name) const;

    std::optional<TrustedHost> get(QModelIndex const & index) const;

    void remove(QModelIndex const & index);

    QVector<TrustedHost> getAll() const;

private:
    QVector<TrustedHost> items;
};

#endif // TRUSTEDHOSTCOLLECTION_HPP
