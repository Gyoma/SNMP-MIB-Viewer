#include "Parser.h"
#include <fstream>


Module::Ptr ModuleTable::findModule(const std::string& name) const
{
    auto res = _modules.find(name);

    if (res != _modules.end())
        return res->second;

    return nullptr;
}

Node::Ptr ModuleTable::findNode(const std::string& Name, const std::string& Module) const
{
    auto module = _modules.find(Module);

    if (module != _modules.end())
    {
        auto const& nodes = module->second->nodes;
        auto node = nodes.find(Name);

        if (node != nodes.end())
            return node->second;
    }

    for (auto const& module : _modules)
    {
        auto const& nodes = module.second->nodes;
        auto node = nodes.find(Name);

        if (node != nodes.end())
            return node->second;
    }

    return nullptr;
}

void ModuleTable::addModule(const Module::Ptr& Module)
{
    if (Module)
        _modules[Module->moduleName] = Module;
}

bool ModuleTable::removeModule(const std::string& Name)
{
    return _modules.erase(Name);
}

void ModuleTable::clear()
{
    _modules.clear();
}

ModuleImport::ModuleImport()
{}

ModuleImport::ModuleImport(const std::string& ModuleName, const Strs& Labels) :
    moduleName(ModuleName),
    labels(Labels)
{}

ModuleImport::ModuleImport(const std::string & ModuleName, Strs && Labels):
    moduleName(ModuleName),
    labels(std::move(Labels))
{}

ModuleImport::ModuleImport(const ModuleImport& other) :
    ModuleImport(other.moduleName, other.labels)
{}

ModuleImport::ModuleImport(ModuleImport&& other) noexcept :
    moduleName(std::move(other.moduleName)),
    labels(std::move(other.labels))
{}

ModuleImport& ModuleImport::operator=(const ModuleImport& other)
{
    if (this != &other)
    {
        moduleName = other.moduleName;
        labels = other.labels;
    }

    return *this;
}

ModuleImport& ModuleImport::operator=(ModuleImport&& other) noexcept
{
    if (this != &other)
    {
        moduleName = std::move(other.moduleName);
        labels = std::move(other.labels);
    }

    return *this;
}

Module::Module(const std::string& ModuleName, const std::string& FileName) :
    moduleName(ModuleName),
    fileName(FileName),
    isParsed(false),
    isLinked(false)
{}

Module::Module(const Module& other) :
    Module(other.moduleName, other.fileName)
{
    imports = other.imports;
    isParsed = other.isParsed;
    isLinked = other.isLinked;
    nodes = other.nodes;
}

Module::Module(Module&& other) noexcept :
moduleName(std::move(other.moduleName)),
imports(std::move(other.imports)),
nodes(std::move(other.nodes)),
fileName(std::move(other.fileName)),
isParsed(other.isParsed),
isLinked(other.isLinked)
{
    other.isParsed = false;
    other.isLinked = false;
}

ModuleMetaData::ModuleMetaData(const std::string& ModuleName, std::string ModulePath,
    bool NeedToLoad, const Strs& ModuleImprots) :
    moduleName(ModuleName),
    modulePath(ModulePath),
    moduleImports(ModuleImprots),
    needToLoad(NeedToLoad)
{}

ModuleMetaData::ModuleMetaData(const ModuleMetaData& other) :
    ModuleMetaData(other.moduleName,
        other.modulePath,
        other.needToLoad,
        other.moduleImports)
{}

ModuleMetaData::ModuleMetaData(ModuleMetaData&& other) noexcept
{
    moduleName = std::move(other.moduleName);
    modulePath = std::move(other.modulePath);
    moduleImports = std::move(other.moduleImports);
    needToLoad = other.needToLoad;
    other.needToLoad = false;
}

ModuleMetaData& ModuleMetaData::operator=(const ModuleMetaData& other)
{
    if (this != &other)
    {
        moduleName = other.moduleName;
        modulePath = other.modulePath;
        needToLoad = other.needToLoad;
    }

    return *this;
}

ModuleMetaData& ModuleMetaData::operator=(ModuleMetaData&& other) noexcept
{
    if (this != &other)
    {
        moduleName = std::move(other.moduleName);
        modulePath = std::move(other.modulePath);
        moduleImports = std::move(other.moduleImports);
        needToLoad = other.needToLoad;
        other.needToLoad = false;
    }

    return *this;
}

ModuleMetaDataTable::ModuleMetaDataTable()
{}

ModuleMetaDataTable::ModuleMetaDataTable(const ModuleMetaDataTable & other) :
    _modulesmeta(other._modulesmeta)
{}

ModuleMetaDataTable::ModuleMetaDataTable(ModuleMetaDataTable && other) noexcept :
    _modulesmeta(std::move(other._modulesmeta))
{}

ModuleMetaDataTable & ModuleMetaDataTable::operator=(const ModuleMetaDataTable & other)
{
    if (this != &other)
        _modulesmeta = other._modulesmeta;

    return *this;
}

ModuleMetaDataTable & ModuleMetaDataTable::operator=(ModuleMetaDataTable && other) noexcept
{
    if (this != &other)
        _modulesmeta = std::move(other._modulesmeta);

    return *this;
}

ModuleMetaData::Ptr ModuleMetaDataTable::findModuleData(const std::string & Name) const
{
    auto res = _modulesmeta.find(Name);

    if (res != _modulesmeta.end())
        return res->second;

    return nullptr;
}

void ModuleMetaDataTable::addModuleData(const ModuleMetaData::Ptr & data)
{
    auto res = _modulesmeta.find(data->moduleName);

    if (res == _modulesmeta.end())
        _modulesmeta[data->moduleName] = data;
}

bool ModuleMetaDataTable::removeModuleData(const std::string & Name)
{
    return _modulesmeta.erase(Name);
}

void ModuleMetaDataTable::clear()
{
    _modulesmeta.clear();
}

ModuleMetaDataTable::ModuleDataVector ModuleMetaDataTable::asVector()
{
    ModuleDataVector res;

    for (auto const& p : _modulesmeta)
        res.push_back(p.second);

    return res;
}
