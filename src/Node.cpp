#include <Node.h>



//Node::Node()
//{}
//
//
//Node::~Node()
//{}

Node::Node() :
    subid(0),
    type(LT::eNA),
    access(LT::eNA),
    status(LT::eNA)
{}

Node::Node(const std::string Name, uint32_t SubID, const std::string& ParentName, const Strs& Modules) :
    Node()
{
    label = Name;
    subid = SubID;
    parentName = ParentName;
    modules = Modules;
}
