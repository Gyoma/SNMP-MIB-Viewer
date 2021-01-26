#pragma once
#include <Node.h>


struct ModuleMetaData
{
    using Ptr = std::shared_ptr<ModuleMetaData>;

    ModuleMetaData(const std::string& ModuleName = "",
        std::string ModulePath = "", 
        bool NeedToLoad = false,
        const Strs& ModuleImprots = {});
    ModuleMetaData(const ModuleMetaData& other);
    ModuleMetaData(ModuleMetaData&& other) noexcept;

    ModuleMetaData& operator=(const ModuleMetaData& other);
    ModuleMetaData& operator=(ModuleMetaData&& other) noexcept;

	std::string     moduleName;
	std::string     modulePath;
    Strs            moduleImports;
	bool            needToLoad;
};

//using ModuleMetaDataTable = std::unordered_map<std::string, ModuleMetaData::Ptr>;

class ModuleMetaDataTable
{
public:

    using Ptr = std::shared_ptr<ModuleMetaDataTable>;
    using ModuleDataVector = std::vector<ModuleMetaData::Ptr>;

    ModuleMetaDataTable();
    ModuleMetaDataTable(const ModuleMetaDataTable& other);
    ModuleMetaDataTable(ModuleMetaDataTable&& other) noexcept;

    ModuleMetaDataTable& operator=(const ModuleMetaDataTable& other);
    ModuleMetaDataTable& operator=(ModuleMetaDataTable&& other) noexcept;

    ModuleMetaData::Ptr findModuleData(const std::string& Name) const;
    void addModuleData(const ModuleMetaData::Ptr& data);
    bool removeModuleData(const std::string& Name);

    void clear();
    ModuleDataVector asVector();

private:

    std::unordered_map<std::string, ModuleMetaData::Ptr> _modulesmeta;
};


struct ModuleImport
{
    ModuleImport();
    ModuleImport(const std::string& ModuleName, const Strs& Labels);
    ModuleImport(const std::string& ModuleName, Strs&& Labels);
    ModuleImport(const ModuleImport& other);
    ModuleImport(ModuleImport&& other) noexcept;

    ModuleImport& operator=(const ModuleImport& other);
    ModuleImport& operator=(ModuleImport&& other) noexcept;

    std::string moduleName;  /* The module imported from */
    Strs labels;  /* The descriptor being imported */
};

using ModuleImportList = std::vector<ModuleImport>;

struct Module
{
    using Ptr = std::shared_ptr<Module>;

    Module(const std::string& ModuleName = "", const std::string& FileName = "");

    Module(const Module& other);
    Module(Module&& other) noexcept;

    const std::string	moduleName;
    std::string			fileName;
    ModuleImportList	imports;
    NodeTable			nodes;
    bool				isParsed;
    bool                isLinked;
};

class ModuleTable
{
public:

    using Ptr = std::shared_ptr<ModuleTable>;
    using ModuleVector = std::vector<Module::Ptr>;

    Module::Ptr findModule(const std::string& Name) const;
    Node::Ptr findNode(const std::string& Name, const std::string& Module = std::string()) const;

    void addModule(const Module::Ptr& Module);
    bool removeModule(const std::string& Name);

    //ModuleVector asVector();
    void clear();

private:

    std::unordered_map<std::string, Module::Ptr> _modules;
};
