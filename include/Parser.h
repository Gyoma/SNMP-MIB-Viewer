#pragma once
#include <Module.h>

class TreeModel;

class Parser
{
    static const LexemTypeTable _lexemTable;

    struct ModuleCompatibility
    {
        std::string oldModule;
        std::string newModule;
        std::string tag;    /* empty string implies unconditional replacement,
                             * otherwise node identifier or prefix
                             */
        bool prefix;
    };

    static const std::vector<ModuleCompatibility> _modCompats;

    enum LoadStatus
    {
        MODULE_NOT_FOUND,
        MODULE_LOADED_OK,
        MODULE_ALREADY_LOADED,
        MODULE_LOAD_FAILED,
        MODULE_SYNTAX_ERROR
    };

    TreeModel* _tree;

    Objgroup _objgroups, _objects, _notifs;
    ErrorInfo _errinf;

    size_t _line;
    std::string _moduleName;

    struct TC
    {
        std::string		descriptor;
        LT				syntax;
        std::string     module;
        std::string		hint;
        std::string     reference;

        EnumList		enums;
        RangeList		ragnes;

        std::string		description;
    };

    using TCList = std::vector<TC>;
    TCList _tclist;


    struct UndefinedNode
    {
        UndefinedNode(const Node::Ptr& Node, const std::string& Syntax);

        Node::Ptr node;
        std::string syntax;
    };

    std::vector<UndefinedNode> _undefNodes;

public:

    using Ptr = std::shared_ptr<Parser>;

    Parser(TreeModel* tree = nullptr);

    NodeList parse(std::ifstream& file);
    void parseToken(std::ifstream& file, Token& token);
    ErrorInfo lastErrorInfo();
    TCList tcList();

    static std::string typeToStr(LT type);
    static Strs getModuleReplacements(const std::string& oldModule);

private:

#define EmptyResult NodeList();

    void parseQuoteString(std::ifstream& file, Token& token);
    bool isLabelChar(char ch);
    void parseImports(std::ifstream& file);
    NodeList parseObjectGroup(std::ifstream& file, const std::string& objName, LT what, Objgroup& objgroup);
    NodeList parseObjType(std::ifstream& file, const std::string& objName);
    NodeList parseNotifType(std::ifstream& file, const std::string& objName);
    NodeList parseCompliance(std::ifstream& file, const std::string& objName);
    NodeList parseModuleIdentity(std::ifstream& file, const std::string& objName);
    NodeList parseCapabilities(std::ifstream& file, const std::string& objName);
    NodeList parseASN(std::ifstream& file, Token& token, const std::string& objName);
    NodeList parseTrap(std::ifstream& file, const std::string& objName);
    NodeList parseMacro(std::ifstream& file);

    NodeList mergeParsedObjectid(Node::Ptr& np, std::ifstream& file);
    NodeList parseObjectid(std::ifstream& file, const std::string& objName);
    std::vector<SubID> parseOIDlist(std::ifstream& file);

    EnumList parseEnums(std::ifstream& file);
    RangeList parseRanges(std::ifstream& file);
    IndexList parseIndexes(std::ifstream& file);
    VarbindList parseVarbinds(std::ifstream& file);

    bool checkUTC(const std::string& utc);
    void tossObjectIdentifier(std::ifstream& file, Token& token);
    void eatSyntax(std::ifstream& file, Token& token);

    bool complianceLookup(const std::string& name, const std::string& moduleName = std::string());

    LoadStatus readModuleInternal(const std::string& moduleName);
    LoadStatus readModuleReplacements(const std::string& oldModuleName);

    void scanObjlist(const NodeList& root, const Module::Ptr& mp, Objgroup& list);
    void resolveSyntax();

    std::string formError(const std::string& str, const std::string& lexem = "");
};