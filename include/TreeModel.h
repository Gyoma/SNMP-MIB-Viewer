#pragma once
#include <QAbstractItemModel>
#include <Module.h>

class Parser;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    using Ptr = std::shared_ptr<TreeModel>;
	using WPtr = std::weak_ptr<TreeModel>;

    TreeModel(const ModuleTable::Ptr& ModuleTable = nullptr);

    Node::Ptr findNode(const std::string& name, const std::string& module = std::string()) const;
    Module::Ptr findModule(const std::string& name) const;

    void linkupNodes(NodeList& nodes);
    void unlinkModule(const std::string& name);
    void linkupModule(const std::string& name);
    void removeModule(const std::string& name);
	void loadModule(const std::string& name);

    void addModuleInfo(const ModuleMetaData::Ptr& data);
	//void updateModuleInfo(const ModuleInfo::Ptr& ModuleInfo);

    QVariant data(const QModelIndex& index, int role) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

private:

//#define ROOTS_MODULE_NAME "#roots#"

    Node* getNode(const QModelIndex& index) const;
    QModelIndex getIndex(const Node::Ptr& node);

    Node::Ptr _header;
    ModuleTable::Ptr _moduleTable;
	std::shared_ptr<Parser> _parser;
};