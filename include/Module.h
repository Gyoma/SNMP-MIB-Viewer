#pragma once
#include "GlobalTypes.h"
#include "Node.h"
#include <string>

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

    Module();
    Module(const std::string& ModuleName, const std::string& FileName);

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

    ModuleTable(const std::string& folderPath);

    Module::Ptr findModule(const std::string& Name) const;
    Node::Ptr findNode(const std::string& Name, const std::string& Module = std::string()) const;

    void addModule(const Module::Ptr& Module);
    bool deleteModule(const std::string& Name);

private:

    std::unordered_map<std::string, Module::Ptr> _modules;
};