#include <QMessageBox>
#include <QIcon>
#include <TreeModel.h>
#include <Parser.h>
#include <fstream>

TreeModel::TreeModel(const ModuleTable::Ptr& moduleTable) :
    _moduleTable(moduleTable),
    _header(new Node("MIB Tree", 0)),
    _parser(new Parser(this))
{
    Node::Ptr ccit(new Node("ccitt", 0));
    ccit->OID = ".0";
    ccit->labelOID = ccit->label;
    Node::Ptr iso(new Node("iso", 1));
    iso->OID = ".1";
    iso->labelOID = iso->label;
    Node::Ptr joint(new Node("joint-iso-ccitt", 2));
    joint->OID = ".2";
    joint->labelOID = joint->label;

    _header->children = { ccit, iso, joint };

    Module::Ptr mibRootsModule(new Module(ROOTS_MODULE_NAME));

    for (auto const& root : _header->children)
        mibRootsModule->nodes[root->label] = root;

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
                        np->OID += parent->OID + '.' + std::to_string(np->subid);
                        np->labelOID += parent->labelOID + '.' + np->label;
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

                    for (auto& moduleName : np->modules)
                    {
                        auto mp = findModule(moduleName);

                        if (mp)
                        {
                            auto const& moduleName = mp->moduleName;

                            if (std::find(existingModules.begin(), existingModules.end(), moduleName) == existingModules.end())
                            {
                                mp->nodes[np->label] = tnp;
                                mp->isLinked = true;
                                existingModules.push_back(moduleName);
                            }
                        }
                    }

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
        emit layoutAboutToBeChanged();

        for (auto const& nodep : mp->nodes)
        {
            auto const& np = nodep.second;
            auto parent = np->parent.lock();

            if (parent) //just in case
            {
                if (np->modules.size() == 1)
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
                else
                {
                    auto& modules = np->modules;
                    modules.erase(std::find(modules.begin(), modules.end(), moduleName));
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

    this->persistentIndexList();

    if (mp && !mp->isLinked)
    {
        emit layoutAboutToBeChanged();

        if (!mp->isParsed)
        {
            loadModule(mp->moduleName);
        }
        else
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
                        auto& pchildren = parent->children;

                        //находим место для вставки
                        auto lower = std::lower_bound(pchildren.begin(), pchildren.end(), np,
                            [](const Node::Ptr& npl, const Node::Ptr& npr)
                            {
                                return npl->subid < npr->subid;
                            });

                        pchildren.insert(lower, np);
                        mp->isLinked = true;
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
        {
            QMessageBox::critical(nullptr,
                QString::fromUtf8("Ошибка загрузки модуля"),
                QString::fromUtf8("Модуль ") +
                QString::fromStdString(name) +
                QString::fromUtf8(" не был найден по указанному пути:\n") +
                QString::QString::fromStdString(mp->fileName)
            );
            return;
        }

        emit layoutAboutToBeChanged();

        _parser->parse(file);

        auto const& errInfo = _parser->lastErrorInfo();

        if (errInfo.isError)
        {
            QMessageBox::critical(nullptr,
                QString::fromUtf8("Ошибка загрузки модуля"),
                QString::fromStdString(errInfo.description));
        }

        emit layoutChanged();
    }
}

void TreeModel::addModuleInfo(const ModuleMetaData::Ptr& data)
{
    if (_moduleTable)
        _moduleTable->addModule(Module::Ptr(new Module(data->moduleName, data->modulePath)));
}

void TreeModel::addModule(const Module::Ptr & module)
{
    if (_moduleTable)
        _moduleTable->addModule(module);
}

void TreeModel::clear()
{
    if (_moduleTable)
    {
        _moduleTable->clear();

        Module::Ptr mibRootsModule(new Module(ROOTS_MODULE_NAME));

        for (auto const& root : _header->children)
        {
            root->children.clear();
            mibRootsModule->nodes[root->label] = root;
        }
       
        _moduleTable->addModule(mibRootsModule);
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
    }

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    Node* parentNode = parent.isValid() ? getNode(parent) : _header.get();

    Node* childNode = parentNode->children[row].get();

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
        return static_cast<Node*>(index.internalPointer());

    return _header.get();
}

QModelIndex TreeModel::getIndex(const Node::Ptr& node)
{
    if (!node || node == _header)
        return QModelIndex();

    return createIndex(node->children.size(), 0, node.get());
}
