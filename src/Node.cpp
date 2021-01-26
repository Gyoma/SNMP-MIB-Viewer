#include <Node.h>

Node::Node() :
    subid(0),
    type(LT::NA),
    syntax(LT::NA),
    access(LT::NA),
    status(LT::NA)
{}

Node::Node(const std::string Name, uint32_t SubID, const std::string& ParentName, const Strs& Modules) :
    Node()
{
    label = Name;
    subid = SubID;
    parentName = ParentName;
    modules = Modules;
}
