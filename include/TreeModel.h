#pragma once
#include <Module.h>
#include <QAbstractItemModel>


class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    using Ptr = std::shared_ptr<TreeModel>;

    TreeModel(const ModuleTable::Ptr& ModuleTable = nullptr);

    Node::Ptr findNode(const std::string& name, const std::string& module = std::string()) const;
    Module::Ptr findModule(const std::string& name) const;

    void linkupNodes(NodeList& nodes);
    void unlinkModule(const std::string& name);
    void linkupModule(const std::string& name);

    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

private:

    Node::Ptr getNode(QModelIndex const& index) const;
    QModelIndex getIndex(Node::Ptr& node);

    Node::Ptr _header;
    ModuleTable::Ptr _moduleTable;
};