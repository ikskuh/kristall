#ifndef DOCUMENTOUTLINEMODEL_HPP
#define DOCUMENTOUTLINEMODEL_HPP

#include <QAbstractItemModel>
#include <QList>

class DocumentOutlineModel :
    public QAbstractItemModel
{
    Q_OBJECT
public:
    DocumentOutlineModel();

    void clear();

    void beginBuild();

    void appendH1(QString const & title, QString const & anchor);

    void appendH2(QString const & title, QString const & anchor);

    void appendH3(QString const & title, QString const & anchor);

    void endBuild();

    QString getTitle(QModelIndex const & index) const;
    QString getAnchor(QModelIndex const & index) const;

public:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;


private:
    struct Node
    {
        Node * parent;
        QString title;
        QString anchor;
        int depth = 0;
        int index = 0;
        QList<Node> children;
    };

    Node root;

    Node & ensureLevel1();
    Node & ensureLevel2();
};

#endif // DOCUMENTOUTLINEMODEL_HPP
