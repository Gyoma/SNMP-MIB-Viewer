#include <Parser.h>
#include <fstream>



ModuleTable::ModuleTable(const Strs& ModulesPaths)
{
    //updateModuleInfo(ModulesPaths);
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

void ModuleTable::updateModuleInfo(const ModuleInfo::Ptr& ModuleInfo)
{
    auto module = findModule(ModuleInfo->moduleName);

    if (module)
    {
        module->fileName = ModuleInfo->modulePath;
    }
    else
        addModule(Module::Ptr(new Module(ModuleInfo->moduleName, ModuleInfo->modulePath)));
}

ModuleImport::ModuleImport()
{}

ModuleImport::ModuleImport(const std::string& ModuleName, const Strs& Labels) :
    moduleName(ModuleName),
    labels(Labels)
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

Module::~Module()
{
    isLinked = false;
}

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

ModuleInfo::ModuleInfo(const std::string& ModuleName, std::string ModulePath, bool NeedToLoad, const Strs& ModuleImprots) :
    moduleName(ModuleName),
    modulePath(ModulePath),
    moduleImports(ModuleImprots),
    needToLoad(NeedToLoad)
{}

ModuleInfo::ModuleInfo(const ModuleInfo& other) :
    ModuleInfo(other.moduleName,
        other.modulePath,
        other.needToLoad,
        other.moduleImports)
{}

ModuleInfo::ModuleInfo(ModuleInfo&& other) noexcept
{
    moduleName = std::move(other.moduleName);
    modulePath = std::move(other.modulePath);
    moduleImports = std::move(other.moduleImports);
    needToLoad = other.needToLoad;
    other.needToLoad = false;
}

ModuleInfo& ModuleInfo::operator=(const ModuleInfo& other)
{
    if (this != &other)
    {
        moduleName = other.moduleName;
        modulePath = other.modulePath;
        needToLoad = other.needToLoad;
    }

    return *this;
}

ModuleInfo& ModuleInfo::operator=(ModuleInfo&& other) noexcept
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
