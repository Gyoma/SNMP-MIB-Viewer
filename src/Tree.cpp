#include "Tree.h"

Tree::Tree(const ModuleTable::Ptr& moduleTable) :
    _moduleTable(moduleTable),
    _root(Node::Ptr(new Node))
{
    _root->children = {
        Node::Ptr(new Node("ccitt", 0)),
        Node::Ptr(new Node("iso", 1)),
        Node::Ptr(new Node("joint-iso-ccitt", 2))
    };

    Module::Ptr mibRootsModule(new Module);

    for (auto const& node : _root->children)
        mibRootsModule->nodes[node->label] = node;

    _moduleTable->addModule(mibRootsModule);
}

Node::Ptr Tree::findNode(const std::string& name, const std::string& module) const
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

Module::Ptr Tree::findModule(const std::string& name) const
{
    return _moduleTable->findModule(name);
}

// links up nodes to the tree
// if the nodes are not empty after executing this method, it means a linking errorg
void Tree::linkupNodes(NodeList& nodes)
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
                                mp->nodes[np->label] = np;
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

void Tree::unlinkModule(const std::string& moduleName)
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

                for(auto const& module : parent->modules)
                    if (module == moduleName)
                    {
                        inSameModule = true;
                        break;
                    }

                if (!inSameModule)
                {
                    auto& children = np->children;

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

void Tree::linkupModule(const std::string& moduleName)
{
    auto mp = findModule(moduleName);

    if (mp)
    {
        for (auto const& nodep : mp->nodes)
        {
            auto const& np = nodep.second;
            auto parent = findNode(np->parentName);

            if (parent)
            {
                np->parent = parent;
                parent->children.push_back(np);

                for (auto const& module : parent->modules)
                {
                    auto parentmp = findModule(module);

                    if (parentmp && parentmp->isLinked)
                    {
                        mp->isLinked = true;
                        break;
                    }
                }
            }
        }
    }
}
