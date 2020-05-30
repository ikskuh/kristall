#ifndef DOCUMENTOUTLINEMODEL_HPP
#define DOCUMENTOUTLINEMODEL_HPP

#include <QAbstractItemModel>

class DocumentOutlineModel :
    public QAbstractItemModel
{
    Q_OBJECT
public:
    DocumentOutlineModel();

    void beginBuild();

    void appendH1(QString const & title);

    void appendH2(QString const & title);

    void appendH3(QString const & title);

    void endBuild();

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
        int depth = 0;
        QVector<Node> children;
    };

    Node root;
};

#endif // DOCUMENTOUTLINEMODEL_HPP
