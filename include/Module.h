#pragma once
#include <Node.h>

struct ModuleInfo
{
    using Ptr = std::shared_ptr<ModuleInfo>;

	ModuleInfo(const std::string& ModuleName = "", 
        std::string ModulePath = "", 
        bool NeedToLoad = false,
        const Strs& ModuleImprots = {});
	ModuleInfo(const ModuleInfo& other);
	ModuleInfo(ModuleInfo&& other) noexcept;

    ModuleInfo& operator=(const ModuleInfo& other);
    ModuleInfo& operator=(ModuleInfo&& other) noexcept;

	std::string moduleName;
	std::string modulePath;
    Strs        moduleImports;
	bool needToLoad = false;
};

using ModuleInfoTable = std::unordered_map<std::string, ModuleInfo::Ptr>;

struct ModuleImport
{
    ModuleImport();
    ModuleImport(const std::string& ModuleName, const Strs& Labels);
    ModuleImport(const ModuleImport& other);
    ModuleImport(ModuleImport&& other) noexcept;

    ModuleImport& operator=(const ModuleImport& other);
    ModuleImport& operator=(ModuleImport&& other) noexcept;

    std::string moduleName;  /* The module imported from */
    Strs labels;  /* The descriptor being imported */
};


struct Module
{
    using Ptr = std::shared_ptr<Module>;
    using Imports = std::vector<ModuleImport>;

    Module(const std::string& ModuleName = "", const std::string& FileName = "");
    ~Module();

    Module(const Module& other);
    Module(Module&& other) noexcept;

    const std::string	moduleName;
    std::string			fileName;
    Imports				imports;
    NodeTable			nodes;
    bool				isParsed;
    bool                isLinked;
};

class ModuleTable
{
public:

    using Ptr = std::shared_ptr<ModuleTable>;

	ModuleTable(const Strs& ModulesPaths = {});

    Module::Ptr findModule(const std::string& Name) const;
    Node::Ptr findNode(const std::string& Name, const std::string& Module = std::string()) const;

    void addModule(const Module::Ptr& Module);
    bool removeModule(const std::string& Name);

	void updateModuleInfo(const ModuleInfo::Ptr& ModuleInfo);

private:

    std::unordered_map<std::string, Module::Ptr> _modules;
};