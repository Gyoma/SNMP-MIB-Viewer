#include "Node.h"
#include "Module.h"
#include <iostream>

class Tree
{
public:

    using Ptr = std::shared_ptr<Tree>;

    Tree(const ModuleTable::Ptr& ModuleTable = nullptr);

    Node::Ptr findNode(const std::string& name, const std::string& module = std::string()) const;
    Module::Ptr findModule(const std::string& name) const;

    void linkUp(NodeList& nodes);

    void print()
    {
        print("", _root, false);
    }

private:

    void print(const std::string& prefix, const Node::Ptr& node, bool isLeft)
    {
        if (node != nullptr)
        {
            std::cout << prefix;

            std::cout << (isLeft ? "|--" : "+--");

            // print the value of the node
            std::cout << (node == _root ? "-+" : node->label) << std::endl;

            // enter the next tree level - left and right branch
            for (int i = 0; i < (int)node->children.size() - 1; ++i)
                print(prefix + (isLeft ? "|   " : "    "), node->children[i], true);

            if(!node->children.empty())
            print(prefix + (isLeft ? "|   " : "    "), node->children.back(), false);
        }
    }

    Node::Ptr _root;
    ModuleTable::Ptr _moduleTable;
};