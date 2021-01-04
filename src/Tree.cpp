#include "..\include\Tree.h"

Tree::Tree(const ModuleTable::Ptr& moduleTable):
	_moduleTable(moduleTable)
{}

Node::Ptr Tree::findNode(const std::string& name, const std::string& module)
{
	return Node::Ptr(new Node()); //todo

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

Module::Ptr Tree::findModule(const std::string& name)
{
	return _moduleTable->findModule(name);
}
