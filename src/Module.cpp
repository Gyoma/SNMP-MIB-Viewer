#include "Parser.h"
#include "Module.h"
#include <iostream>
#include <filesystem>
#include <fstream>



ModuleTable::ModuleTable(const std::string& folderPath)
{
	
	for (const auto& entry : std::filesystem::directory_iterator(folderPath))
	{
		std::ifstream file(entry.path());
		Parser parser;
		Token token;
		Module module;

		if (file.is_open())
		{
			parser.parseToken(file, token);

			_modules[token.lexem] = Module::Ptr(new Module(token.lexem, entry.path().string(), {}));

		}

	}

}

Module::Ptr ModuleTable::findModule(const std::string& name)
{
	auto res = _modules.find(name);

	if (res != _modules.end())
		return res->second;

	return nullptr;
}

void ModuleTable::addModule(const Module::Ptr& module)
{
	if (module)
		_modules[module->moduleName] = module;
}

bool ModuleTable::deleteModule(const std::string& name)
{
	return _modules.erase(name);
}

ModuleImport::ModuleImport()
{}

ModuleImport::ModuleImport(const std::string& Module, const Strs& Labels) :
	module(Module),
	labels(Labels)
{}

ModuleImport::ModuleImport(const ModuleImport& other) :
	ModuleImport(other.module, other.labels)
{}

ModuleImport::ModuleImport(ModuleImport&& other) noexcept :
	module(std::move(other.module)),
	labels(std::move(other.labels))
{}


Module::Module()
{}

Module::Module(const std::string& Module, const std::string& FileName, const std::vector<ModuleImport>& Imports) :
	moduleName(Module),
	fileName(FileName),
	imports(Imports)
{}

Module::Module(const Module& other) :
	Module(other.moduleName, other.fileName, other.imports)
{}

Module::Module(Module&& other) noexcept :
	moduleName(std::move(other.moduleName)),
	imports(std::move(other.imports)),
	fileName(std::move(other.fileName))
{}
