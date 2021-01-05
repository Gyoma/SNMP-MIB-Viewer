#include "Node.h"
#include "Module.h"

class Tree
{
public:

    using Ptr = std::shared_ptr<Tree>;

    Tree(const ModuleTable::Ptr& ModuleTable = nullptr);

    Node::Ptr findNode(const std::string& name, const std::string& module = std::string()) const;
    Module::Ptr findModule(const std::string& name) const;

    void linkUp(NodeList& nodes);

private:

    Node::Ptr _root;
    ModuleTable::Ptr _moduleTable;
};