#include "Node.h"
#include "Module.h"

class Tree
{
public:

	using Ptr = std::shared_ptr<Tree>;

	Tree(const ModuleTable::Ptr & ModuleTable = nullptr);

	Node::Ptr findNode(const std::string& name, const std::string& module = std::string());
	Module::Ptr findModule(const std::string& name);

private:

	Node::Ptr _root;
	std::unordered_map<std::string, Node::Ptr> _nodeTable;
	ModuleTable::Ptr _moduleTable;
};