#pragma once
#include "GlobalTypes.h"
#include <string>

struct ModuleImport
{
	ModuleImport();
	ModuleImport(const std::string& Module, const Strs& Labels);
	ModuleImport(const ModuleImport& other);
	ModuleImport(ModuleImport&& other) noexcept;

	std::string module;  /* The module imported from */
	Strs labels;  /* The descriptor being imported */
};


struct Module
{
	using Ptr = std::shared_ptr<Module>;
	using Imports = std::vector<ModuleImport>;

	Module();
	Module(const std::string& Module, const std::string& FileName, 
		const std::vector<ModuleImport>& Imports);
	Module(const Module& other);
	Module(Module&& other) noexcept;

	const std::string	moduleName;
	std::string			fileName;
	Imports				imports;
};

class ModuleTable
{
public:
	
	using Ptr = std::shared_ptr<ModuleTable>;

	ModuleTable(const std::string& folderPath);
	
	Module::Ptr findModule(const std::string& name);
	void addModule(const Module::Ptr& module);
	bool deleteModule(const std::string& name);

private:

	std::unordered_map<std::string, Module::Ptr> _modules;
};