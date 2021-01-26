#pragma once
#include <GlobalTypes.h>

struct Node
{
    using Ptr = std::shared_ptr<Node>;
    using WPtr = std::weak_ptr<Node>;

    Node();
    Node(const std::string Name, uint32_t SubID, 
        const std::string& ParentName = "", const Strs& Modules = {});


    std::string					label;  /* This node's (unique) textual name */
    uint32_t					subid;  /* This node's integer subidentifier */
    std::string                 OID;
    std::string                 labelOID;
    std::vector<Ptr>			children;
    Strs						modules;  /* The module containing this node */
    std::string					parentName; /* The parent's textual name */
    WPtr						parent;
    LT							type;   /* The type of object this represents */
    LT                          syntax;
    LT							access;
    LT							status;
    EnumList					enums; /* (optional) list of enumerated integers */
    RangeList					ranges;
    IndexList					indexes;
    VarbindList					varbinds;
    std::string					augments;
    std::string					hint;
    std::string					units;
    std::string					description; /* description (a quoted string) */
    std::string					reference; /* references (a quoted string) */
    std::string					defaultValue;
};

using NodeTable = std::unordered_map<std::string, Node::Ptr>;