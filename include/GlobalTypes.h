#pragma once
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <vector>
#include <memory>
#include <string>


#define ROOTS_MODULE_NAME "MIB ROOTS"

//Lexem types
enum LT
{
    //Keywords

    CONTINUE = -1,
    DEFINITIONS,
    BEGIN,
    END,
    NA,
    ENDOFFILE,
    IMPORTS,
    EXPORTS,
    FROM,
    EQUALS,
    LEFTPAREN,
    RIGHTPAREN,
    LEFTBRACKET,
    RIGHTBRACKET,
    LEFTSQBRACK,
    RIGHTSQBRACK,
    SEMI,
    COMMA,
    BAR,
    RANGE,
    LASTUPDATED,
    ORGANIZATION,
    CONTACTINFO,
    AUGMENTS,
    UNITS,
    REFERENCE,
    OF,
    SEQUENCE,
    DESCRIPTION,
    INDEX,
    DEFVAL,
    SIZE,
    MAXACCESS,
    ACCESS,
    MINACCESS,
    STATUS,
    SYNTAX,
    ENTERPRISE,
    DISPLAYHINT,
    IMPLIED,
    SUPPORTS,
    INCLUDES,
    VARIATION,
    REVISION,
    OBJECTS,
    MODULE,
    PRODREL,
    WRSYNTAX,
    CREATEREQ,
    MANDATORYGROUPS,
    GROUP,
    CHOICE,
    IMPLICIT,

    //Types

    OTHER,
    LABEL,
    OBSOLETE,
    OPTIONAL,
    MODULEIDENTITY,
    NOACCESS,
    WRITEONLY,
    NUMENTRIES,
    NOTIFTYPE,
    OBJGROUP,
    COMPLIANCE,
    IDENTIFIER,
    OBJECT,
    READWRITE,
    READCREATE,
    READONLY,
    DEPRECATED,
    MANDATORY,
    CURRENT,
    OBJTYPE,
    TRAPTYPE,
    ACCNOTIFY,
    CONVENTION,
    NOTIFGROUP,
    AGENTCAP,
    MACRO,
    NOTIMPL,
    NOTIFICATIONS,
    NUMBER,
    QUOTESTRING,

    SYNTAX_MASK = 0x80,
    OBJID,
    OCTETSTR,
    INTEGER,
    NETADDR,
    IPADDR,
    COUNTER,
    GAUGE,
    TIMETICKS,
    OPAQUE,
    NUL,
    BITSTRING,
    NSAPADDRESS,
    COUNTER64,
    UINTEGER32,
    APPSYNTAX,
    OBJSYNTAX,
    SIMPLESYNTAX,
    OBJNAME,
    NOTIFNAME,
    VARIABLES,
    UNSIGNED32,
    INTEGER32,
    OBJIDENTITY
};


using Strs = std::vector<std::string>;
using LexemTypeTable = std::unordered_map<std::string, LT>;
using NodeList = std::list<std::shared_ptr<struct Node>>;
using Objgroup = std::unordered_set<std::string>;

struct Token
{
    Token(size_t lexemCapacity = 128) :
        type(LT::NA)
    {
        lexem.reserve(lexemCapacity);
    }

    std::string lexem;
    LT type;
};

struct SubID
{
    std::string		label;
    int             subid;
};

struct ErrorInfo
{
    bool isError = false;
    std::string description;
};

/*
* A linked list of tag-value pairs for enumerated integers.
*/
struct Enum
{
    std::string		label;
    int             value;
};

using EnumList = std::vector<Enum>;

/*
* A linked list of ranges
*/
struct Range
{
    int	low, high;
};

using RangeList = std::vector<Range>;

/*
* A linked list of indexes
*/
struct Index
{
    std::string ilabel;
    bool		isimplied;
};

using IndexList = std::vector<Index>;

/*
* A linked list of varbinds
*/
using VarbindList = Strs;