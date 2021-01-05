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

            _modules[token.lexem] = Module::Ptr(new Module(token.lexem, entry.path().string(), {}, false));

        }

    }

}

Module::Ptr ModuleTable::findModule(const std::string& name) const
{
    auto res = _modules.find(name);

    if (res != _modules.end())
        return res->second;

    return nullptr;
}

Node::Ptr ModuleTable::findNode(const std::string& Name, const std::string& Module) const
{
    if (Module.empty())
    {
        for (auto const& module : _modules)
        {
            auto const& nodes = module.second->nodes;
            auto node = nodes.find(Name);

            if (node != nodes.end())
                return node->second;
        }
    }
    else
    {
        auto module = _modules.find(Module);

        if (module != _modules.end())
        {
            auto const& nodes = module->second->nodes;
            auto node = nodes.find(Name);

            if (node != nodes.end())
                return node->second;
        }
    }

    return nullptr;
}

void ModuleTable::addModule(const Module::Ptr& Module)
{
    if (Module)
        _modules[Module->moduleName] = Module;
}

bool ModuleTable::deleteModule(const std::string& Name)
{
    return _modules.erase(Name);
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


Module::Module() :
    isParsed(false)
{}

Module::Module(const std::string& Module, const std::string& FileName, const std::vector<ModuleImport>& Imports, bool IsParsed) :
    moduleName(Module),
    fileName(FileName),
    imports(Imports),
    isParsed(IsParsed)
{}

Module::Module(const Module& other) :
    Module(other.moduleName, other.fileName, other.imports, other.isParsed)
{}

Module::Module(Module&& other) noexcept :
    moduleName(std::move(other.moduleName)),
    imports(std::move(other.imports)),
    fileName(std::move(other.fileName)),
    isParsed(other.isParsed)
{
    other.isParsed = false;
}
