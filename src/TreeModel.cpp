#include <TreeModel.h>

TreeModel::TreeModel(const ModuleTable::Ptr& moduleTable) :
    _moduleTable(moduleTable),
    _header(Node::Ptr(new Node("MIB Tree", 0)))
{
    _header->children = {
        Node::Ptr(new Node("ccitt", 0)),
        Node::Ptr(new Node("iso", 1)),
        Node::Ptr(new Node("joint-iso-ccitt", 2))
    };

    Module::Ptr mibRootsModule(new Module);

    for (auto const& node : _header->children)
        mibRootsModule->nodes[node->label] = node;

    _moduleTable->addModule(mibRootsModule);
}

Node::Ptr TreeModel::findNode(const std::string& name, const std::string& module) const
{
    //return Node::Ptr(new Node());

    return _moduleTable->findNode(name, module);

    //auto res = _nodeTable.find(name);
    //
    //if (res != _nodeTable.end())
    //{
    //	if (module.empty())
    //		return res->second;
    //	else
    //	{
    //		auto const& node = res->second;
    //		auto const& modules = node->modules;
    //		size_t size = modules.size();
    //
    //		for (size_t i = 0; i < size; ++i)
    //			if (modules[i] == module)
    //				return node;
    //	}
    //}
    //
    //return nullptr;
}

Module::Ptr TreeModel::findModule(const std::string& name) const
{
    return _moduleTable->findModule(name);
}

// links up nodes to the tree
// if the nodes are not empty after executing this method, it means a linking errorg
void TreeModel::linkupNodes(NodeList& nodes)
{
    //in the process of parsing nodes, they will be ordered and one could use this, 
    //but this approach is generic

    bool success = true;

    while (success)
    {
        success = false;

        for (auto nodeit = nodes.begin(); nodeit != nodes.end(); )
        {
            auto const& np = *nodeit;

            //check that the node is not connected yet

            auto tnp = findNode(np->label);

            if (tnp == nullptr || tnp->parent.expired())
            {
                auto parent = findNode(np->parentName);

                if (parent)
                {
                    np->parent = parent;
                    parent->children.push_back(np);

                    if (tnp == nullptr)
                    {
                        for (auto& moduleName : np->modules)
                        {
                            auto mp = findModule(moduleName);

                            if (mp)
                            {
                                mp->nodes[np->label] = np;
                                mp->isLinked = true;
                            }
                        }
                    }

                    nodeit = nodes.erase(nodeit);
                    success = true;
                }
                else
                    ++nodeit;
            }
            else
            {
                auto& existingModules = tnp->modules;
                auto const& moduleName = np->modules.front();
                size_t size = existingModules.size(), i = 0;

                for (; i < size; ++i)
                {
                    if (existingModules[i] == moduleName)
                        break;
                }

                if (i == size)
                    existingModules.push_back(moduleName);

                nodeit = nodes.erase(nodeit);
            }
        }
    }
}

void TreeModel::unlinkModule(const std::string& moduleName)
{
    auto mp = findModule(moduleName);

    if (mp && mp->isLinked)
    {
        for (auto const& nodep : mp->nodes)
        {
            auto const& np = nodep.second;
            auto parent = np->parent.lock();

            if (parent) //just in case
            {
                bool inSameModule = false;

                for (auto const& module : parent->modules)
                    if (module == moduleName)
                    {
                        inSameModule = true;
                        break;
                    }

                if (!inSameModule)
                {
                    auto& children = parent->children;

                    for (auto it = children.begin(); it != children.end(); ++it)
                        if ((*it)->label == np->label)
                        {
                            children.erase(it);
                            break;
                        }

                    np->parent.reset();
                }
            }
        }

        mp->isLinked = false;
    }
}

void TreeModel::linkupModule(const std::string& moduleName)
{
    auto mp = findModule(moduleName);

    if (mp)
    {
        for (auto const& nodep : mp->nodes)
        {
            auto const& np = nodep.second;

            if (np->parent.lock() == nullptr)
            {
                auto parent = findNode(np->parentName);

                if (parent)
                {
                    np->parent = parent;
                    parent->children.push_back(np);

                    for (auto const& module : parent->modules)
                    {
                        auto parentmp = findModule(module);

                        if (parentmp)
                        {
                            if (!parentmp->isLinked)
                                linkupModule(module);

                            if (parentmp->isLinked)
                            {
                                mp->isLinked = true;
                                //break;
                            }
                        }
                    }
                }
            }
        }
    }
}

QVariant TreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const Node::Ptr& node = getNode(index);

    switch (role)
    {
    case Qt::DisplayRole:
        return QString::fromStdString(node->label);
    //case Qt::DecorationRole:
    //    if (index.column() == 0)
    //        return item->getIcon();
    //    break;
    }

    return QVariant();
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        //switch (section)
        //{
        //case 0:
        return QString::fromStdString(_header->label);
        //case 1:
        //    return _header->inf();
        //}
    }

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    //TreeItem* parentItem;
    const Node::Ptr& parentNode = parent.isValid() ? getNode(parent) : _header;



    //if (!parent.isValid())
    //    parentNode = _header;
    ////parentItem = _header;
    //else
    //    parentNode = getNode(parent);
        //parentItem = getItem(parent);

    Node::Ptr& childNode = parentNode->children[row];
    //TreeItem* childItem = parentItem->child(row);
    if (childNode)
        return createIndex(row, column, &childNode);

    return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    const Node::Ptr& childNode = getNode(index);
    Node::Ptr& parentNode = childNode ? childNode->parent.lock() : nullptr;

    if (parentNode == _header || !parentNode)
        return QModelIndex();

    return createIndex(parentNode->children.size(), 0, &parentNode);
}

int TreeModel::rowCount(const QModelIndex& parent) const
{
    const Node::Ptr& parentNode = getNode(parent);
    return parentNode ? parentNode->children.size() : 0;
}

int TreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

Node::Ptr TreeModel::getNode(QModelIndex const& index) const
{
    if (index.isValid())
    {
        return *static_cast<Node::Ptr*>(index.internalPointer());


        //Node::Ptr* node = static_cast<Node::Ptr*>(index.internalPointer());
        //
        //if (node)
        //    return *node;
    }
    return _header;
}

QModelIndex TreeModel::getIndex(Node::Ptr& node)
{
    if (!node || node == _header)
        return QModelIndex();

    return createIndex(node->children.size(), 0, &node);
}
