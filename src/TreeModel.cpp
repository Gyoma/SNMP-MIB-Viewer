#include <TreeModel.h>
#include <Parser.h>
#include <fstream>

TreeModel::TreeModel(const ModuleTable::Ptr& moduleTable) :
    _moduleTable(moduleTable),
    _header(new Node("MIB Tree", 0)),
    _parser(new Parser(this))
{
    _header->children = {
        Node::Ptr(new Node("ccitt", 0, "", { ROOTS_MODULE_NAME })),
        Node::Ptr(new Node("iso", 1, "", { ROOTS_MODULE_NAME })),
        Node::Ptr(new Node("joint-iso-ccitt", 2, "", { ROOTS_MODULE_NAME }))
    };

    Module::Ptr mibRootsModule(new Module(ROOTS_MODULE_NAME));

    for (auto const& node : _header->children)
        mibRootsModule->nodes[node->label] = node;

    if (_moduleTable)
        _moduleTable->addModule(mibRootsModule);
}

Node::Ptr TreeModel::findNode(const std::string& name, const std::string& module) const
{
    return _moduleTable ? _moduleTable->findNode(name, module) : nullptr;
}

Module::Ptr TreeModel::findModule(const std::string& name) const
{
    return _moduleTable ? _moduleTable->findModule(name) : nullptr;
}

// links up nodes to the tree
// if the nodes are not empty after executing this method, it means a linking errorg
void TreeModel::linkupNodes(NodeList& nodes)
{
    //in the process of parsing nodes, they will be ordered and one could use this, 
    //but this approach is generic

    if (_moduleTable)
    {
        bool success = true;

        while (success)
        {
            success = false;

            for (auto nodeit = nodes.begin(); nodeit != nodes.end(); )
            {
                auto const& np = *nodeit;

                //check that the node is not connected yet
                auto tnp = findNode(np->label, np->modules.front());

                if (tnp == nullptr)
                {
                    auto parent = findNode(np->parentName);

                    if (parent)
                    {
                        np->parent = parent;
                        auto& pchildren = parent->children;

                        //находим место для вставки
                        auto lower = std::lower_bound(pchildren.begin(), pchildren.end(), np,
                            [](const Node::Ptr& npl, const Node::Ptr& npr)
                            {
                                return npl->subid < npr->subid;
                            });

                        pchildren.insert(lower, np);

                        for (auto& moduleName : np->modules)
                        {
                            auto mp = findModule(moduleName);

                            if (mp)
                            {
                                mp->nodes[np->label] = np;
                                mp->isLinked = true;
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
        emit layoutChanged();
    }
}

void TreeModel::linkupModule(const std::string& moduleName)
{
    auto mp = findModule(moduleName);

    if (mp && !mp->isLinked)
    {
        if (!mp->isParsed)
        {
            loadModule(mp->moduleName);
            emit layoutChanged();
            return;
        }

        for (auto const& nodep : mp->nodes)
        {
            auto const& np = nodep.second;

            if (np->parent.lock() == nullptr)
            {
                auto parent = findNode(np->parentName);

                if (parent)
                {
                    np->parent = parent;
                    auto& pchildren = parent->children;

                    //находим место для вставки
                    auto lower = std::lower_bound(pchildren.begin(), pchildren.end(), np,
                        [](const Node::Ptr& npl, const Node::Ptr& npr)
                        {
                            return npl->subid < npr->subid;
                        });

                    pchildren.insert(lower, np);

                    for (auto const& module : parent->modules)
                    {
                        auto parentmp = findModule(module);

                        if (parentmp)
                        {
                            if (!parentmp->isLinked)
                                linkupModule(module);

                            mp->isLinked = true;
                        }
                    }
                }
            }
        }
        emit layoutChanged();
    }
}

void TreeModel::removeModule(const std::string& name)
{
    auto mp = findModule(name);

    if (mp)
    {
        for (auto const& nodep : mp->nodes)
        {
            auto const& np = nodep.second;

            auto parent = np->parent.lock();

            if (parent)
            {
                auto& children = parent->children;

                for (auto childit = children.begin(); childit != children.end(); ++childit)
                {
                    if (*childit == np)
                    {
                        children.erase(childit);
                        break;
                    }
                }
            }
        }

        _moduleTable->removeModule(mp->moduleName);
        emit layoutChanged();
    }
}

void TreeModel::loadModule(const std::string& name)
{

    auto mp = findModule(name);

    if (mp)
    {
        std::ifstream file(mp->fileName);

        if (!file.is_open())
            return;

        _parser->parse(file);
        emit layoutChanged();
    }
}

void TreeModel::updateModuleInfo(const ModuleInfo::Ptr& ModuleInfo)
{
    if (_moduleTable)
    {
        _moduleTable->updateModuleInfo(ModuleInfo);
    }
}

QVariant TreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Node* node = getNode(index);

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
        return QString::fromUtf8(u8"MIB Дерево");
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
    Node* parentNode = parent.isValid() ? getNode(parent) : _header.get();



    //if (!parent.isValid())
    //    parentNode = _header;
    ////parentItem = _header;
    //else
    //    parentNode = getNode(parent);
        //parentItem = getItem(parent);

    Node* childNode = parentNode->children[row].get();
    //TreeItem* childItem = parentItem->child(row);
    if (childNode)
        return createIndex(row, column, childNode);

    return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    Node* childNode = getNode(index);
    Node* parentNode = childNode ? childNode->parent.lock().get() : nullptr;

    if (parentNode == _header.get() || !parentNode)
        return QModelIndex();

    return createIndex(parentNode->children.size(), 0, parentNode);
}

int TreeModel::rowCount(const QModelIndex& parent) const
{
    Node* parentNode = getNode(parent);
    return parentNode ? parentNode->children.size() : 0;
}

int TreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

Node* TreeModel::getNode(const QModelIndex& index) const
{
    if (index.isValid())
    {
        return static_cast<Node*>(index.internalPointer());


        //Node::Ptr* node = static_cast<Node::Ptr*>(index.internalPointer());
        //
        //if (node)
        //    return *node;
    }
    return _header.get();
}

QModelIndex TreeModel::getIndex(const Node::Ptr& node)
{
    if (!node || node == _header)
        return QModelIndex();

    return createIndex(node->children.size(), 0, node.get());
}
