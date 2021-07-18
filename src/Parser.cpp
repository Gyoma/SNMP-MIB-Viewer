#include "Parser.h"
#include "TreeModel.h"
#include <fstream>


const LexemTypeTable Parser::_lexemTable = {
        { "obsolete",				LT::OBSOLETE },
        { "Opaque",					LT::OPAQUE },
        { "optional" ,				LT::OPTIONAL },
        { "LAST-UPDATED",			LT::LASTUPDATED },
        { "ORGANIZATION",			LT::ORGANIZATION },
        { "CONTACT-INFO",			LT::CONTACTINFO },
        { "MODULE-IDENTITY",		LT::MODULEIDENTITY },
        { "MODULE-COMPLIANCE",		LT::COMPLIANCE },
        { "DEFINITIONS",			LT::DEFINITIONS },
        { "END",					LT::END },
        { "AUGMENTS",				LT::AUGMENTS },
        { "not-accessible",			LT::NOACCESS },
        { "write-only",				LT::WRITEONLY },
        { "NsapAddress",			LT::NSAPADDRESS },
        { "UNITS",					LT::UNITS },
        { "REFERENCE",				LT::REFERENCE },
        { "NUM-ENTRIES" ,			LT::NUMENTRIES },
        { "BITSTRING",				LT::BITSTRING },
        { "BIT",					LT::CONTINUE },
        { "BITS",					LT::BITSTRING },
        { "Counter64",				LT::COUNTER64 },
        { "TimeTicks",				LT::TIMETICKS },
        { "NOTIFICATION-TYPE",		LT::NOTIFTYPE },
        { "OBJECT-GROUP" ,			LT::OBJGROUP },
        { "OBJECT-IDENTITY",		LT::OBJIDENTITY },
        { "IDENTIFIER",				LT::IDENTIFIER },
        { "OBJECT",					LT::OBJECT },
        { "NetworkAddress",			LT::NETADDR },
        { "Gauge",					LT::GAUGE },
        { "Gauge32",				LT::GAUGE },
        { "Unsigned32",				LT::UNSIGNED32 },
        { "read-write",				LT::READWRITE },
        { "read-create",			LT::READCREATE },
        { "OCTETSTRING",			LT::OCTETSTR },
        { "OCTET",					LT::CONTINUE },
        { "OF",						LT::OF },
        { "SEQUENCE",				LT::SEQUENCE },
        { "NULL",					LT::NUL },
        { "IpAddress",				LT::IPADDR },
        { "UInteger32",				LT::UINTEGER32 },
        { "INTEGER",				LT::INTEGER },
        { "Integer32",				LT::INTEGER32 },
        { "Counter",				LT::COUNTER },
        { "Counter32",				LT::COUNTER },
        { "read-only",				LT::READONLY },
        { "DESCRIPTION",			LT::DESCRIPTION },
        { "INDEX",					LT::INDEX },
        { "DEFVAL",					LT::DEFVAL },
        { "deprecated",				LT::DEPRECATED },
        { "SIZE",					LT::SIZE },
        { "MAX-ACCESS",				LT::ACCESS },
        { "ACCESS",					LT::ACCESS },
        { "mandatory",				LT::MANDATORY },
        { "current",				LT::CURRENT },
        { "STATUS",					LT::STATUS },
        { "SYNTAX",					LT::SYNTAX },
        { "OBJECT-TYPE",			LT::OBJTYPE },
        { "TRAP-TYPE",				LT::TRAPTYPE },
        { "ENTERPRISE",				LT::ENTERPRISE },
        { "BEGIN",					LT::BEGIN },
        { "IMPORTS",				LT::IMPORTS },
        { "EXPORTS",				LT::EXPORTS },
        { "accessible-for-notify",	LT::ACCNOTIFY },
        { "TEXTUAL-CONVENTION",		LT::CONVENTION },
        { "NOTIFICATION-GROUP",		LT::NOTIFGROUP },
        { "DISPLAY-HINT",			LT::DISPLAYHINT },
        { "FROM",					LT::FROM },
        { "AGENT-CAPABILITIES",		LT::AGENTCAP },
        { "MACRO",					LT::MACRO },
        { "IMPLIED",				LT::IMPLIED },
        { "SUPPORTS",				LT::SUPPORTS },
        { "INCLUDES",				LT::INCLUDES },
        { "VARIATION",				LT::VARIATION },
        { "REVISION",				LT::REVISION },
        { "not-implemented",		LT::NOTIMPL },
        { "OBJECTS",				LT::OBJECTS },
        { "NOTIFICATIONS",			LT::NOTIFICATIONS },
        { "MODULE",					LT::MODULE },
        { "MIN-ACCESS",				LT::MINACCESS },
        { "PRODUCT-RELEASE",		LT::PRODREL },
        { "WRITE-SYNTAX",			LT::WRSYNTAX },
        { "CREATION-REQUIRES",		LT::CREATEREQ },
        { "MANDATORY-GROUPS",		LT::MANDATORYGROUPS },
        { "GROUP",					LT::GROUP },
        { "CHOICE",					LT::CHOICE },
        { "IMPLICIT",				LT::IMPLICIT },
        { "ObjectSyntax",			LT::OBJSYNTAX },
        { "SimpleSyntax",			LT::SIMPLESYNTAX },
        { "ApplicationSyntax",		LT::APPSYNTAX },
        { "ObjectName",				LT::OBJNAME },
        { "NotificationName",		LT::NOTIFNAME },
        { "VARIABLES",				LT::VARIABLES }
};

const std::vector<Parser::ModuleCompatibility> Parser::_modCompats = {

        { "RFC1065-SMI", "RFC1155-SMI", "", false },
        { "RFC1066-MIB", "RFC1156-MIB", "", false },
        /*
         * 'mib' -> 'mib-2'
         */
        { "RFC1156-MIB", "RFC1158-MIB", "", false },
        /*
         * 'snmpEnableAuthTraps' -> 'snmpEnableAuthenTraps'
         */
        { "RFC1158-MIB", "RFC1213-MIB", "", false },
        /*
         * 'nullOID' -> 'zeroDotZero'
         */
        { "RFC1155-SMI", "SNMPv2-SMI", "", false },
        { "RFC1213-MIB", "SNMPv2-SMI", "mib-2", false },
        { "RFC1213-MIB", "SNMPv2-MIB", "sys", true },
        { "RFC1213-MIB", "IF-MIB", "if", true },
        { "RFC1213-MIB", "IP-MIB", "ip", true },
        { "RFC1213-MIB", "IP-MIB", "icmp", true },
        { "RFC1213-MIB", "TCP-MIB", "tcp", true },
        { "RFC1213-MIB", "UDP-MIB", "udp", true },
        { "RFC1213-MIB", "SNMPv2-SMI", "transmission", false },
        { "RFC1213-MIB", "SNMPv2-MIB", "snmp", true },
        { "RFC1231-MIB", "TOKENRING-MIB", "", false },
        { "RFC1271-MIB", "RMON-MIB", "", false },
        { "RFC1286-MIB", "SOURCE-ROUTING-MIB", "dot1dSr", true },
        { "RFC1286-MIB", "BRIDGE-MIB", "", false },
        { "RFC1315-MIB", "FRAME-RELAY-DTE-MIB", "", false },
        { "RFC1316-MIB", "CHARACTER-MIB", "", false },
        { "RFC1406-MIB", "DS1-MIB", "", false },
        { "RFC-1213", "RFC1213-MIB", "", false }
};

Parser::Parser(TreeModel* Tree) :
    _tree(Tree),
    _line(0)
{}

bool Parser::isLabelChar(char ch)
{
    if ((isalnum(ch)) || (ch == '-'))
        return true;

    if (ch == '_')
    {
        return true;
    }

    return false;
}

void Parser::parseImports(std::ifstream& file)
{
    Token token;
    ModuleImport import;
    ModuleImportList imports;

    parseToken(file, token);

    while (token.type != LT::SEMI && token.type != LT::ENDOFFILE)
    {
        if (token.type == LT::LABEL)
        {
            import.labels.push_back(token.lexem);
        }
        else if (token.type == LT::FROM)
        {
            parseToken(file, token);

            if (import.labels.empty())
            {
                parseToken(file, token);
                continue;
            }

            import.moduleName = token.lexem;
            imports.push_back(std::move(import));

            auto status = readModuleInternal(token.lexem);

            if (status == LoadStatus::MODULE_NOT_FOUND)
            {
                status = readModuleReplacements(token.lexem);

                if (status == LoadStatus::MODULE_NOT_FOUND)
                {
                    _errinf.isError = true;
                    _errinf.description = "Module " + token.lexem + " not found.";
                    return;
                }
            }

            if (_errinf.isError)
                return;
        }
        parseToken(file, token);
    }

    auto& module = _tree ? _tree->findModule(_moduleName) : nullptr;

    if (module)
    {
        if (imports.empty())
            return;

        module->imports = std::move(imports);

        return;
    }

    _errinf.isError = true;
    _errinf.description = "Module " + _moduleName + " not found.";
}

NodeList Parser::parseObjectGroup(std::ifstream& file, const std::string& objName, LT what, Objgroup& objgroup)
{
    Token token;
    Node::Ptr np(new Node);
    np->label = objName;

    parseToken(file, token);

    if (token.type == what)
    {
        parseToken(file, token);

        if (token.type != LT::LEFTBRACKET)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected \"{\"", token.lexem);
            return EmptyResult;
        }

        do
        {
            parseToken(file, token);
            if (token.type != LT::LABEL)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad identifier", token.lexem);
                return EmptyResult;
            }

            objgroup.insert(token.lexem);
            parseToken(file, token);

        } while (token.type == LT::COMMA);

        if (token.type != LT::RIGHTBRACKET)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected \"}\"", token.lexem);
            return EmptyResult;
        }
        parseToken(file, token);
    }

    if (token.type != LT::STATUS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected STATUS", token.lexem);
        return EmptyResult;
    }

    parseToken(file, token);

    if (token.type != LT::CURRENT && token.type != LT::DEPRECATED && token.type != LT::OBSOLETE)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad STATUS value", token.lexem);
        return EmptyResult;
    }

    np->status = token.type;

    parseToken(file, token);

    if (token.type != LT::DESCRIPTION)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected DESCRIPTION", token.lexem);
        return EmptyResult;
    }
    parseToken(file, token);

    if (token.type != LT::QUOTESTRING)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad DESCRIPTION", token.lexem);
        return EmptyResult;
    }

    np->description = token.lexem;

    parseToken(file, token);

    if (token.type == LT::REFERENCE)
    {
        parseToken(file, token);

        if (token.type != LT::QUOTESTRING)
        {
            _errinf.isError = true;
            _errinf.description = formError("Bad REFERENCE", token.lexem);
            return EmptyResult;
        }
        np->reference = token.lexem;
        parseToken(file, token);
    }

    if (token.type != LT::EQUALS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected \"::=\"", token.lexem);
        return EmptyResult;
    }

    //skip:
    while (token.type != LT::EQUALS && token.type != LT::ENDOFFILE)
        parseToken(file, token);

    return mergeParsedObjectid(np, file);
}

NodeList Parser::parseObjType(std::ifstream& file, const std::string& objName)
{
    Token token;
    Node::Ptr np(new Node);
    np->label = objName;

    parseToken(file, token);

    if (token.type != LT::SYNTAX)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad format for OBJECT-TYPE", token.lexem);
        return EmptyResult;
    }

    parseToken(file, token);

    if (token.type == LT::OBJECT)
    {
        parseToken(file, token);

        if (token.type != LT::IDENTIFIER)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected IDENTIFIER", token.lexem);
            return EmptyResult;
        }
        token.type = LT::OBJID;
    }

    if (token.type == LT::LABEL)
    {
        _undefNodes.emplace_back(np, token.lexem);
    }

    np->syntax = token.type;

    LT lasttype = token.type;

    parseToken(file, token);

    switch (lasttype)
    {
    case LT::SEQUENCE:

        if (token.type == LT::OF)
        {
            parseToken(file, token);
            parseToken(file, token);

        }
        break;
    case LT::INTEGER:
    case LT::INTEGER32:
    case LT::UINTEGER32:
    case LT::UNSIGNED32:
    case LT::COUNTER:
    case LT::GAUGE:
    case LT::BITSTRING:
    case LT::LABEL:

        if (token.type == LT::LEFTBRACKET)
        {
            np->enums = parseEnums(file);

            if (np->enums.empty())
            {
                return EmptyResult;
            }

            parseToken(file, token);
        }
        else if (token.type == LT::LEFTPAREN)
        {
            np->ranges = parseRanges(file);

            if (np->ranges.empty())
            {
                return EmptyResult;
            }

            parseToken(file, token);
        }
        break;
    case LT::OCTETSTR:
    case LT::OPAQUE:
        if (token.type == LT::LEFTPAREN)
        {
            parseToken(file, token);

            if (token.type == LT::SIZE)
            {
                parseToken(file, token);

                if (token.type == LT::LEFTPAREN)
                {
                    np->ranges = parseRanges(file);

                    if (_errinf.isError)
                        return EmptyResult;

                    parseToken(file, token);
                    if (token.type == LT::RIGHTPAREN)
                    {
                        parseToken(file, token);
                        break;
                    }
                }
            }

            _errinf.isError = true;
            _errinf.description = formError("Bad SIZE syntax", token.lexem);
            return EmptyResult;
        }
        break;
    case LT::OBJID:
    case LT::NETADDR:
    case LT::IPADDR:
    case LT::TIMETICKS:
    case LT::NUL:
    case LT::NSAPADDRESS:
    case LT::COUNTER64:
        break;
    default:
        _errinf.isError = true;
        _errinf.description = formError("Bad syntax", token.lexem);
        return EmptyResult;
    }

    if (token.type == LT::UNITS)
    {
        parseToken(file, token);

        if (token.type != LT::QUOTESTRING)
        {
            _errinf.isError = true;
            _errinf.description = formError("Bad UNITS", token.lexem);
            return EmptyResult;
        }
        np->units = token.lexem;
        parseToken(file, token);
    }

    if (token.type != LT::ACCESS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Should be ACCESS", token.lexem);
        return EmptyResult;
    }

    parseToken(file, token);


    if (token.type != LT::READONLY && token.type != LT::READWRITE
        && token.type != LT::WRITEONLY && token.type != LT::NOACCESS
        && token.type != LT::READCREATE && token.type != LT::ACCNOTIFY)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad ACCESS type", token.lexem);
        return EmptyResult;
    }

    np->access = token.type;

    parseToken(file, token);

    if (token.type != LT::STATUS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Should be STATUS", token.lexem);
        return EmptyResult;
    }

    parseToken(file, token);

    if (token.type != LT::MANDATORY && token.type != LT::CURRENT
        && token.type != LT::OPTIONAL && token.type != LT::OBSOLETE
        && token.type != LT::DEPRECATED)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad STATUS", token.lexem);
        return EmptyResult;
    }

    np->status = token.type;
    /*
    * Optional parts of the OBJECT-TYPE macro
    */
    parseToken(file, token);

    while (token.type != LT::EQUALS && token.type != LT::ENDOFFILE)
    {
        switch (token.type)
        {
        case LT::DESCRIPTION:

            parseToken(file, token);

            if (token.type != LT::QUOTESTRING)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad DESCRIPTION", token.lexem);
                return EmptyResult;
            }

            np->description = token.lexem;
            break;

        case LT::REFERENCE:

            parseToken(file, token);
            if (token.type != LT::QUOTESTRING)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad REFERENCE", token.lexem);
                return EmptyResult;
            }
            np->reference = token.lexem;
            break;
        case LT::INDEX:

            if (!np->augments.empty())
            {
                _errinf.isError = true;
                _errinf.description = formError("Cannot have both INDEX and AUGMENTS", token.lexem);
                return EmptyResult;
            }

            np->indexes = parseIndexes(file);

            if (np->indexes.empty())
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad INDEX list", token.lexem);
                return EmptyResult;
            }

            break;
        case LT::AUGMENTS:

            if (!np->indexes.empty())
            {
                _errinf.isError = true;
                _errinf.description = formError("Cannot have both INDEX and AUGMENTS", token.lexem);
                return EmptyResult;
            }

            np->indexes = parseIndexes(file);

            if (np->indexes.empty())
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad AUGMENTS list", token.lexem);
                return EmptyResult;
            }

            np->augments = np->indexes.front().ilabel;
            break;

        case LT::DEFVAL:

            parseToken(file, token);
            if (token.type != LT::LEFTBRACKET)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad DEFAULTVALUE", token.lexem);
                return EmptyResult;
            }

            {
                int level = 1;
                std::string defbuf;

                while (true)
                {
                    parseToken(file, token);

                    if ((token.type == LT::RIGHTBRACKET && --level == 0)
                        || token.type == LT::ENDOFFILE)
                        break;
                    else if (token.type == LT::LEFTBRACKET)
                        level++;

                    defbuf += token.lexem + ' ';

                }
                defbuf.pop_back();

                if (token.type != LT::RIGHTBRACKET)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Bad DEFAULTVALUE", token.lexem);
                    return EmptyResult;
                }

                np->defaultValue = defbuf;
            }

            break;

        case LT::NUMENTRIES:

            tossObjectIdentifier(file, token);

            if (token.type != LT::OBJID)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad Object Identifier", token.lexem);
                return EmptyResult;
            }

            break;

        default:

            _errinf.isError = true;
            _errinf.description = formError("Bad format of optional clauses", token.lexem);
            return EmptyResult;

        }
        parseToken(file, token);
    }
    if (token.type != LT::EQUALS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad format", token.lexem);
        return EmptyResult;
    }
    return mergeParsedObjectid(np, file);
}

NodeList Parser::parseNotifType(std::ifstream& file, const std::string& objName)
{
    Node::Ptr np(new Node);
    np->label = objName;

    Token token;

    parseToken(file, token);

    while (token.type != LT::EQUALS && token.type != LT::ENDOFFILE) //mb
    {
        switch (token.type)
        {
        case LT::STATUS:

            parseToken(file, token);


            if (token.type != LT::CURRENT && token.type != LT::DEPRECATED && token.type != LT::OBSOLETE)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad STATUS value", token.lexem);
                return EmptyResult;
            }

            np->status = token.type;

            break;
        case LT::DESCRIPTION:

            parseToken(file, token);

            if (token.type != LT::QUOTESTRING)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad DESCRIPTION", token.lexem);
                return EmptyResult;
            }

            np->description = token.lexem;

            break;
        case LT::REFERENCE:
            parseToken(file, token);
            if (token.type != LT::QUOTESTRING)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad REFERENCE", token.lexem);
                return EmptyResult;
            }

            np->reference = token.lexem;
            break;
        case LT::OBJECTS:

            np->varbinds = parseVarbinds(file);

            if (np->varbinds.empty())
            {
                if (_errinf.isError == false)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Bad OBJECTS list", token.lexem);
                }
                return EmptyResult;
            }
            break;
        default:

            _errinf.isError = true;
            _errinf.description = formError("Unexpected", token.lexem);
            return EmptyResult;
        }

        parseToken(file, token);
    }
    return mergeParsedObjectid(np, file);
}


NodeList Parser::parseCompliance(std::ifstream& file, const std::string& objName)
{

    Node::Ptr np(new Node);
    np->label = objName;
    Token token;
    std::string moduleName;
    int debug = 0;

    parseToken(file, token);

    if (token.type != LT::STATUS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected STATUS", token.lexem);
        return EmptyResult;

    }

    parseToken(file, token);

    if (token.type != LT::CURRENT && token.type != LT::DEPRECATED && token.type != LT::OBSOLETE)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad STATUS", token.lexem);
        return EmptyResult;
    }
    np->status = token.type;

    parseToken(file, token);

    if (token.type != LT::DESCRIPTION)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected DESCRIPTION", token.lexem);
        return EmptyResult;
    }

    parseToken(file, token);

    if (token.type != LT::QUOTESTRING)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad DESCRIPTION", token.lexem);
        return EmptyResult;
    }
    np->description = token.lexem;

    parseToken(file, token);

    if (token.type == LT::REFERENCE)
    {
        parseToken(file, token);

        if (token.type != LT::QUOTESTRING)
        {
            _errinf.isError = true;
            _errinf.description = formError("Bad REFERENCE", token.lexem);
            return EmptyResult;
        }

        np->reference = token.lexem;
        parseToken(file, token);
    }

    if (token.type != LT::MODULE)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected MODULE", token.lexem);
        return EmptyResult;
    }

    while (token.type == LT::MODULE)
    {
        std::string module;

        parseToken(file, token);

        if (token.type == LT::LABEL && token.lexem != _moduleName)
        {
            moduleName = token.lexem;

            LoadStatus status = readModuleInternal(moduleName);

            if (status != LoadStatus::MODULE_LOADED_OK && status != LoadStatus::MODULE_ALREADY_LOADED)
            {
                _errinf.isError = true;
                _errinf.description = formError("Unknown module", token.lexem);
                return EmptyResult;
            }

            parseToken(file, token);
        }

        if (token.type == LT::MANDATORYGROUPS)
        {
            parseToken(file, token);

            if (token.type != LT::LEFTBRACKET)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected \"{\"", token.lexem);
                return EmptyResult;
            }
            do
            {
                parseToken(file, token);

                if (token.type != LT::LABEL)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Bad group name", token.lexem);
                    return EmptyResult;
                }

                if (!complianceLookup(token.lexem, moduleName))
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Unknown group", token.lexem)
                        + "\nTry to load " + moduleName + " first";
                    return EmptyResult;
                }

                parseToken(file, token);

            } while (token.type == LT::COMMA);

            if (token.type != LT::RIGHTBRACKET)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected \"}\"", token.lexem);
                return EmptyResult;
            }

            parseToken(file, token);
        }

        while (token.type == LT::GROUP || token.type == LT::OBJECT)
        {
            if (token.type == LT::GROUP)
            {
                parseToken(file, token);

                if (token.type != LT::LABEL)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Bad group name", token.lexem);
                    return EmptyResult;
                }

                if (!complianceLookup(token.lexem, moduleName))
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Unknown group", token.lexem)
                        + "\nTry to load " + moduleName + " first";
                    return EmptyResult;
                }

                parseToken(file, token);
            }
            else
            {
                parseToken(file, token);

                if (token.type != LT::LABEL)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Bad object name", token.lexem);
                    return EmptyResult;
                }

                if (!complianceLookup(token.lexem, moduleName))
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Unknown group", token.lexem)
                        + "\nTry to load " + moduleName + " first";
                    return EmptyResult;
                }

                parseToken(file, token);

                if (token.type == LT::SYNTAX)
                {
                    eatSyntax(file, token);

                    if (_errinf.isError)
                        return EmptyResult;
                }

                if (token.type == LT::WRSYNTAX)
                {
                    eatSyntax(file, token);

                    if (_errinf.isError)
                        return EmptyResult;
                }

                if (token.type == LT::MINACCESS)
                {

                    parseToken(file, token);

                    if (token.type != LT::NOACCESS && token.type != LT::ACCNOTIFY
                        && token.type != LT::READONLY && token.type != LT::WRITEONLY
                        && token.type != LT::READCREATE && token.type != LT::READWRITE)
                    {
                        _errinf.isError = true;
                        _errinf.description = formError("Bad MIN-ACCESS spec", token.lexem);
                        return EmptyResult;
                    }

                    parseToken(file, token);
                }
            }

            if (token.type != LT::DESCRIPTION)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected DESCRIPTION", token.lexem);
                return EmptyResult;
            }

            parseToken(file, token);
            if (token.type != LT::QUOTESTRING)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad DESCRIPTION", token.lexem);
                return EmptyResult;
            }

            parseToken(file, token);
        }

        moduleName.clear();
    }

    return mergeParsedObjectid(np, file);
}

NodeList Parser::parseModuleIdentity(std::ifstream& file, const std::string& objName)
{
    Node::Ptr np(new Node);
    np->label = objName;

    Token token;

    parseToken(file, token);

    if (token.type != LT::LASTUPDATED)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected LAST-UPDATED", token.lexem);
        return EmptyResult;
    }

    parseToken(file, token);
    if (token.type != LT::QUOTESTRING)
    {
        _errinf.isError = true;
        _errinf.description = formError("Need STRING for LAST-UPDATED", token.lexem);
        return EmptyResult;
    }

    if (!checkUTC(token.lexem))
        return EmptyResult;

    parseToken(file, token);

    if (token.type != LT::ORGANIZATION)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected ORGANIZATION", token.lexem);
        return EmptyResult;
    }

    parseToken(file, token);

    if (token.type != LT::QUOTESTRING)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad ORGANIZATION", token.lexem);
        return EmptyResult;
    }

    parseToken(file, token);

    if (token.type != LT::CONTACTINFO)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected CONTACT-INFO", token.lexem);
        return EmptyResult;
    }

    parseToken(file, token);

    if (token.type != LT::QUOTESTRING)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad CONTACT-INFO", token.lexem);
        return EmptyResult;
    }

    parseToken(file, token);

    if (token.type != LT::DESCRIPTION)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected DESCRIPTION", token.lexem);
        return EmptyResult;
    }

    parseToken(file, token);

    if (token.type != LT::QUOTESTRING)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad DESCRIPTION", token.lexem);
        return EmptyResult;
    }

    np->description = token.lexem;

    parseToken(file, token);

    while (token.type == LT::REVISION)
    {
        parseToken(file, token);

        if (token.type != LT::QUOTESTRING)
        {
            _errinf.isError = true;
            _errinf.description = formError("Bad REVISION", token.lexem);
            return EmptyResult;
        }

        if (!checkUTC(token.lexem))
            return EmptyResult;

        parseToken(file, token);

        if (token.type != LT::DESCRIPTION)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected DESCRIPTION", token.lexem);
            return EmptyResult;
        }

        parseToken(file, token);

        if (token.type != LT::QUOTESTRING)
        {
            _errinf.isError = true;
            _errinf.description = formError("Bad DESCRIPTION", token.lexem);
            return EmptyResult;
        }

        parseToken(file, token);
    }

    if (token.type != LT::EQUALS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected \"::=\"", token.lexem);
        return EmptyResult;
    }

    return mergeParsedObjectid(np, file);
}

NodeList Parser::parseCapabilities(std::ifstream& file, const std::string& objName)
{
    Node::Ptr np(new Node);
    np->label = objName;

    Token token;

    parseToken(file, token);

    if (token.type != LT::PRODREL)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected PRODUCT-RELEASE", token.lexem);
        return EmptyResult;
    }

    parseToken(file, token);

    if (token.type != LT::QUOTESTRING)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected STRING after PRODUCT-RELEASE", token.lexem);
        return EmptyResult;
    }

    parseToken(file, token);

    if (token.type != LT::STATUS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected STATUS", token.lexem);
        return EmptyResult;
    }

    parseToken(file, token);

    if (token.type != LT::CURRENT && token.type != LT::OBSOLETE)
    {
        _errinf.isError = true;
        _errinf.description = formError("STATUS should be current or obsolete", token.lexem);
        return EmptyResult;
    }

    np->status = token.type;

    parseToken(file, token);

    if (token.type != LT::DESCRIPTION)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected DESCRIPTION", token.lexem);
        return EmptyResult;
    }

    parseToken(file, token);

    if (token.type != LT::QUOTESTRING)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad DESCRIPTION", token.lexem);
        return EmptyResult;
    }

    np->description = token.lexem;

    parseToken(file, token);

    if (token.type == LT::REFERENCE)
    {
        parseToken(file, token);

        if (token.type != LT::QUOTESTRING)
        {
            _errinf.isError = true;
            _errinf.description = formError("Bad REFERENCE", token.lexem);
            return EmptyResult;
        }
        np->reference = token.lexem;

        parseToken(file, token);
    }

    while (token.type == LT::SUPPORTS)
    {
        std::string module;

        parseToken(file, token);

        if (token.type != LT::LABEL)
        {
            _errinf.isError = true;
            _errinf.description = formError("Bad module name", token.lexem);
            return EmptyResult;
        }

        LoadStatus status = readModuleInternal(token.lexem);

        if (status != LoadStatus::MODULE_LOADED_OK && status != LoadStatus::MODULE_ALREADY_LOADED)
        {
            _errinf.isError = true;
            _errinf.description = formError("Module not found", token.lexem);
            return EmptyResult;
        }

        module = token.lexem;

        parseToken(file, token);

        if (token.type != LT::INCLUDES)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected INCLUDES", token.lexem);
            return EmptyResult;
        }

        parseToken(file, token);

        if (token.type != LT::LEFTBRACKET)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected \"{\"", token.lexem);
            return EmptyResult;
        }
        do
        {
            parseToken(file, token);

            if (token.type != LT::LABEL)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected group name", token.lexem);
                return EmptyResult;
            }

            if ((_tree ? _tree->findNode(token.lexem, module) : nullptr) == nullptr)
            {
                _errinf.isError = true;
                _errinf.description = formError("Group not found", token.lexem);
                return EmptyResult;
            }

            parseToken(file, token);

        } while (token.type == LT::COMMA);

        if (token.type != LT::RIGHTBRACKET)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected \"}\" after group list", token.lexem);
            return EmptyResult;
        }

        parseToken(file, token);

        while (token.type == LT::VARIATION)
        {
            parseToken(file, token);

            if (token.type != LT::LABEL)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad object name", token.lexem);
                return EmptyResult;
            }

            if ((_tree ? _tree->findNode(token.lexem, module) : nullptr) == nullptr)
            {
                _errinf.isError = true;
                _errinf.description = formError("Object not found in module", token.lexem);
                return EmptyResult;
            }

            parseToken(file, token);

            if (token.type == LT::SYNTAX)
            {
                eatSyntax(file, token);

                if (_errinf.isError)
                    return EmptyResult;

            }
            if (token.type == LT::WRSYNTAX)
            {
                eatSyntax(file, token);

                if (_errinf.isError)
                    return EmptyResult;

            }
            if (token.type == LT::ACCESS)
            {
                parseToken(file, token);

                if (token.type != LT::ACCNOTIFY && token.type != LT::READONLY
                    && token.type != LT::READWRITE && token.type != LT::READCREATE
                    && token.type != LT::WRITEONLY && token.type != LT::NOTIMPL)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Bad ACCESS", token.lexem);
                    return EmptyResult;
                }

                parseToken(file, token);
            }

            if (token.type == LT::CREATEREQ)
            {
                parseToken(file, token);

                if (token.type != LT::LEFTBRACKET)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Expected \"{\"", token.lexem);
                    return EmptyResult;
                }
                do
                {
                    parseToken(file, token);

                    if (token.type != LT::LABEL)
                    {
                        _errinf.isError = true;
                        _errinf.description = formError("Bad object name in list", token.lexem);
                        return EmptyResult;
                    }

                    parseToken(file, token);

                } while (token.type == LT::COMMA);

                if (token.type != LT::RIGHTBRACKET)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Expected \"}\" after list", token.lexem);
                    return EmptyResult;
                }

                parseToken(file, token);
            }

            if (token.type == LT::DEFVAL)
            {
                int             level = 1;
                parseToken(file, token);

                if (token.type != LT::LEFTBRACKET)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Expected \"{\" after DEFVAL", token.lexem);
                    return EmptyResult;
                }

                do
                {
                    parseToken(file, token);

                    if (token.type == LT::LEFTBRACKET)
                        level++;
                    else if (token.type == LT::RIGHTBRACKET)
                        level--;

                } while ((token.type != LT::RIGHTBRACKET || level != 0) && token.type != LT::ENDOFFILE);

                if (token.type != LT::RIGHTBRACKET)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Missing \"}\" after DEFVAL", token.lexem);
                    return EmptyResult;
                }

                parseToken(file, token);
            }

            if (token.type != LT::DESCRIPTION)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected DESCRIPTION", token.lexem);
                return EmptyResult;
            }

            parseToken(file, token);

            if (token.type != LT::QUOTESTRING)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad DESCRIPTION", token.lexem);
                return EmptyResult;
            }

            parseToken(file, token);
        }
    }

    if (token.type != LT::EQUALS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected \"::=\"", token.lexem);
        return EmptyResult;
    }

    return mergeParsedObjectid(np, file);
}

NodeList Parser::parseASN(std::ifstream& file, Token& token, const std::string& objName)
{
    TC tc;

    parseToken(file, token);

    if (token.type == LT::SEQUENCE || token.type == LT::CHOICE)
    {
        int level = 0;
        LT lasttype = token.type;

        parseToken(file, token);
        while (token.type != LT::ENDOFFILE)
        {
            if (token.type == LT::LEFTBRACKET)
            {
                level++;
            }
            else if (token.type == LT::RIGHTBRACKET && --level == 0)
            {
                tc.descriptor = objName;
                tc.syntax = lasttype;
                tc.module = _moduleName;
                _tclist.push_back(tc);

                parseToken(file, token);
                return EmptyResult;
            }
            parseToken(file, token);
        }

        _errinf.isError = true;
        _errinf.description = formError("Expected \"}\"", token.lexem);
    }
    else if (token.type == LT::LEFTBRACKET)
    {
        file.unget();
        auto nodes = parseObjectid(file, objName);

        if (!nodes.empty())
        {
            parseToken(file, token);
            return nodes;
        }
    }
    else if (token.type == LT::LEFTSQBRACK)
    {
        bool size = false;

        do
        {
            parseToken(file, token);

        } while (token.type != LT::ENDOFFILE && token.type != LT::RIGHTSQBRACK);

        if (token.type != LT::RIGHTSQBRACK)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected \"]\"", token.lexem);
            return EmptyResult;
        }

        parseToken(file, token);

        if (token.type == LT::IMPLICIT)
            parseToken(file, token);

        LT lasttype = token.type;

        parseToken(file, token);

        if (token.type == LT::LEFTPAREN)
        {
            switch (lasttype)
            {
            case LT::OCTETSTR:

                parseToken(file, token);

                if (token.type != LT::SIZE)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Expected SIZE", token.lexem);
                    return EmptyResult;
                }
                size = true;
                parseToken(file, token);

                if (token.type != LT::LEFTPAREN)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Expected \"(\"", token.lexem);
                    return EmptyResult;
                }

                /* FALL THROUGH */
            case LT::INTEGER:
                parseToken(file, token);

                do
                {
                    if (token.type != LT::NUMBER)
                    {
                        _errinf.isError = true;
                        _errinf.description = formError("Expected NUMBER", token.lexem);
                        return EmptyResult;
                    }

                    parseToken(file, token);

                    if (token.type == LT::RANGE)
                    {
                        parseToken(file, token);

                        if (token.type != LT::NUMBER)
                        {
                            _errinf.isError = true;
                            _errinf.description = formError("Expected NUMBER", token.lexem);
                            return EmptyResult;
                        }

                        parseToken(file, token);
                    }
                } while (token.type == LT::BAR);

                if (token.type != LT::RIGHTPAREN)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Expected \")\"", token.lexem);
                    return EmptyResult;
                }

                parseToken(file, token);

                if (size)
                {
                    if (token.type != LT::RIGHTPAREN)
                    {
                        _errinf.isError = true;
                        _errinf.description = formError("Expected \")\"", token.lexem);
                        return EmptyResult;
                    }

                    parseToken(file, token);
                }
            }
        }
        //return EmptyResult;
    }
    else
    {
        if (token.type == LT::CONVENTION)
        {
            while (token.type != LT::SYNTAX && token.type != LT::ENDOFFILE)
            {
                if (token.type == LT::DISPLAYHINT)
                {
                    parseToken(file, token);

                    if (token.type != LT::QUOTESTRING)
                    {
                        _errinf.isError = true;
                        _errinf.description = formError("DISPLAY-HINT must be string", token.lexem);
                        return EmptyResult;
                    }

                    tc.hint = token.lexem;
                }
                else if (token.type == LT::DESCRIPTION)
                {
                    parseToken(file, token);

                    if (token.type != LT::QUOTESTRING)
                    {
                        _errinf.isError = true;
                        _errinf.description = formError("DESCRIPTION must be string", token.lexem);
                        return EmptyResult;
                    }

                    tc.description = token.lexem;
                }
                else if (token.type == LT::REFERENCE)
                {
                    parseToken(file, token);

                    if (token.type != LT::QUOTESTRING)
                    {
                        _errinf.isError = true;
                        _errinf.description = formError("REFERENCE must be string", token.lexem);
                        return EmptyResult;
                    }

                    tc.reference = token.lexem;
                }
                else
                    parseToken(file, token);
            }

            parseToken(file, token);

            if (token.type == LT::OBJECT)
            {
                parseToken(file, token);

                if (token.type != LT::IDENTIFIER)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Expected IDENTIFIER", token.lexem);
                    return EmptyResult;
                }

                token.type = LT::OBJID;
            }
        }
        else if (token.type == LT::OBJECT)
        {
            parseToken(file, token);

            if (token.type != LT::IDENTIFIER)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected IDENTIFIER", token.lexem);
                return EmptyResult;
            }

            token.type = LT::OBJID;
        }

        if (token.type == LT::LABEL)
        {

            for (size_t i = 0, size = _tclist.size(); i < size; ++i)
            {
                auto const& tc = _tclist[i];

                if (tc.descriptor == token.lexem)
                {
                    token.type = tc.syntax;
                    break;
                }
            }
        }

        if (!(token.type & LT::SYNTAX_MASK))
        {
            _errinf.isError = true;
            _errinf.description = formError("Textual convention doesn't map to real type", token.lexem);
            return EmptyResult;
        }

        tc.module = _moduleName;
        tc.descriptor = objName;
        tc.syntax = token.type;

        parseToken(file, token);

        if (token.type == LT::LEFTPAREN)
        {
            tc.ragnes = parseRanges(file);
            parseToken(file, token);
        }
        else if (token.type == LT::LEFTBRACKET)
        {
            tc.enums = parseEnums(file);
            parseToken(file, token);
        }

        _tclist.push_back(tc);
    }

    return EmptyResult;
}

NodeList Parser::parseTrap(std::ifstream& file, const std::string& objName)
{
    Node::Ptr np(new Node);
    np->label = objName;
    np->modules.push_back(_moduleName);

    Token token;
    parseToken(file, token);

    while (token.type != LT::EQUALS && token.type != LT::ENDOFFILE)
    {
        switch (token.type)
        {
        case LT::DESCRIPTION:
            parseToken(file, token);

            if (token.type != LT::QUOTESTRING)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad DESCRIPTION", token.lexem);
                return EmptyResult;
            }

            np->description = token.lexem;

            break;
        case LT::REFERENCE:

            parseToken(file, token);

            if (token.type != LT::QUOTESTRING)
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad REFERENCE", token.lexem);
                return EmptyResult;
            }
            np->reference = token.lexem;

            break;
        case LT::ENTERPRISE:

            parseToken(file, token);

            if (token.type == LT::LEFTBRACKET)
            {
                parseToken(file, token);

                if (token.type != LT::LABEL)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Bad ENTERPRISE format", token.lexem);
                    return EmptyResult;
                }

                np->parentName = token.lexem;
                /*
                * Get right bracket
                */
                parseToken(file, token);
            }
            else if (token.type == LT::LABEL)
            {
                np->parentName = token.lexem;
            }
            else
            {
                _errinf.isError = true;
                _errinf.description = formError("Bad ENTERPRISE format", token.lexem);
                return EmptyResult;
            }
            break;
        case LT::VARIABLES:

            np->varbinds = parseVarbinds(file);

            if (np->varbinds.empty())
            {
                if (_errinf.isError == false)
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Bad VARIABLES list", token.lexem);
                }
                return EmptyResult;
            }
            break;
        default:
            break;
        }

        parseToken(file, token);
    }

    parseToken(file, token);

    if (token.type != LT::NUMBER)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected a NUMBER", token.lexem);
        return EmptyResult;
    }

    np->subid = strtoul(token.lexem.c_str(), nullptr, 10);

    if (np->parentName.empty())
    {
        return EmptyResult;
    }

    return NodeList({ np });
}

NodeList Parser::parseMacro(std::ifstream& file)
{
    Node::Ptr np(new Node);
    Token token;

    parseToken(file, token);

    while (token.type != LT::EQUALS && token.type != LT::BEGIN && token.type != LT::ENDOFFILE)
    {
        parseToken(file, token);
    }

    if (token.type != LT::EQUALS)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected \"::=\"", token.lexem);
        return EmptyResult;
    }

    while (token.type != LT::BEGIN && token.type != LT::ENDOFFILE)
    {
        parseToken(file, token);
    }

    if (token.type != LT::BEGIN)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected BEGIN", token.lexem);
        return EmptyResult;
    }

    while (token.type != LT::END && token.type != LT::ENDOFFILE)
    {
        parseToken(file, token);
    }

    if (token.type != LT::END)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected END", token.lexem);
        return EmptyResult;
    }

    return NodeList({ np });
}

NodeList Parser::mergeParsedObjectid(Node::Ptr& np, std::ifstream& file)
{
    auto res = parseObjectid(file, np->label);

    if (res.empty())
    {
        np.reset();
        return res;
    }

    auto& last = res.back();
    np->parentName = last->parentName;
    np->subid = last->subid;
    np->modules = std::move(last->modules);
    last = np;

    return res;
}

NodeList Parser::parseObjectid(std::ifstream& file, const std::string& objName)
{

    Node::Ptr np = nullptr;
    auto oids = parseOIDlist(file);

    if (_errinf.isError)
        return EmptyResult;

    NodeList res;

    // счетчик анонимных узлов
    // случай: rptrInfoHealth  OBJECT IDENTIFIER ::= { snmpDot3RptrMgt 0 4 }
    static unsigned int anonymous = 0;


    if (oids.empty())
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad object identifier");
        return res;
    }

    if (oids[0].label.empty())
    {
        if (oids.size() == 1)
        {
            _errinf.isError = true;
            _errinf.description = formError("Attempt to define a root oid");
            return res;
        }

        auto& oid = oids[0];

        auto const& mibRootsModule = _tree ? _tree->findModule(ROOTS_MODULE_NAME) : nullptr;

        if (mibRootsModule == nullptr)
        {
            _errinf.isError = true;
            _errinf.description = "Tree is not set";
            return res;
        }

        auto const& roots = mibRootsModule->nodes;

        for (auto const& root : roots)
        {
            if (root.second->subid == oid.subid)
            {
                oid.label = root.first;
                break;
            }
        }
    }

    if (oids.size() == 1)
    {
        np = Node::Ptr(new Node);
        np->subid = oids[0].subid;
        np->label = oids[0].label;
        np->modules.push_back(_moduleName);
        res.push_back(np);
        return res;
    }

    size_t size = oids.size();
    for (size_t i = 0; i < size - 1; ++i)
    {
        auto& noid = oids[i + 1];
        np = Node::Ptr(new Node);


        np->parentName = oids[i].label;
        np->modules.push_back(_moduleName);

        if (i == size - 2)
        {
            np->label = objName;
        }
        else
        {
            if (noid.label.empty())
            {
                noid.label = "anonymous#" + std::to_string(anonymous++);
            }
            np->label = noid.label;
        }

        if (noid.subid != -1)
            np->subid = noid.subid;
        else
        {
            res.clear();
            break;
        }

        res.push_back(np);
    }

    return res;
}

std::vector<SubID> Parser::parseOIDlist(std::ifstream& file)
{
    std::vector<SubID> res;
    SubID sub;
    Token token;

    parseToken(file, token);

    if (token.type != LT::LEFTBRACKET)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected \"{\"", token.lexem);
        return res;
    }

    parseToken(file, token);

    while (token.type != LT::RIGHTBRACKET && token.type != LT::ENDOFFILE)
    {
        sub.label.clear();
        sub.subid = -1;
        if (token.type == LT::RIGHTBRACKET)
            return res;
        //return count;
        if (token.type == LT::LABEL)
        {
            /*
            * this entry has a label
            */
            sub.label = token.lexem;
            parseToken(file, token);

            if (token.type == LT::LEFTPAREN)
            {
                parseToken(file, token);

                if (token.type == LT::NUMBER)
                {
                    sub.subid = strtoul(token.lexem.c_str(), nullptr, 10);

                    parseToken(file, token);

                    if (token.type != LT::RIGHTPAREN)
                    {
                        _errinf.isError = true;
                        _errinf.description = formError("Expected \")\"", token.lexem);
                        res.clear();
                        return res;
                    }

                    parseToken(file, token);
                }
                else
                {
                    _errinf.isError = true;
                    _errinf.description = formError("Expected a number", token.lexem);
                    res.clear();
                    return res;
                }
            }
            //else
            //{
            //	continue;
            //}
        }
        else if (token.type == LT::NUMBER)
        {
            sub.subid = strtoul(token.lexem.c_str(), nullptr, 10);
            parseToken(file, token);
        }
        else
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected label or number", token.lexem);
            res.clear();
            return res;
        }

        res.push_back(sub);
    }

    if (token.type == LT::ENDOFFILE)
    {
        _errinf.isError = true;
        _errinf.description = formError("Too long OID", token.lexem);
        res.clear();
    }

    return res;
}

EnumList Parser::parseEnums(std::ifstream& file)
{

    Token token;
    EnumList res;

    parseToken(file, token);

    while (token.type != LT::ENDOFFILE)
    {
        if (token.type == LT::RIGHTBRACKET)
            break;
        /* some enums use "deprecated" to indicate a no longer value label */
        /* (EG: IP-MIB's IpAddressStatusTC) */
        if (token.type == LT::LABEL || token.type == LT::DEPRECATED)
        {
            Enum enm;
            enm.label = token.lexem;

            parseToken(file, token);

            if (token.type != LT::LEFTPAREN)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected \"(\"", token.lexem);
                return res;
            }

            parseToken(file, token);

            if (token.type != LT::NUMBER)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected integer", token.lexem);
                return res;
            }
            enm.value = strtol(token.lexem.c_str(), nullptr, 10);

            parseToken(file, token);

            if (token.type != LT::RIGHTPAREN)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected \")\"", token.lexem);
                return res;
            }
            res.push_back(enm);
        }

        parseToken(file, token);
    }

    if (token.type == LT::ENDOFFILE)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected \"}\"", token.lexem);
        return res;
    }

    return res;
}

RangeList Parser::parseRanges(std::ifstream& file)
{

    Token token;
    Range rng;
    RangeList res;
    bool size = false, taken = true;

    parseToken(file, token);

    if (token.type == LT::SIZE)
    {
        size = true;
        taken = false;

        parseToken(file, token);

        if (token.type != LT::LEFTPAREN)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected \"(\" after SIZE", token.lexem);
            return res;
        }
    }

    do
    {
        if (!taken)
            parseToken(file, token);
        else
            taken = false;

        rng.high = rng.low = strtoul(token.lexem.c_str(), nullptr, 10);

        parseToken(file, token);

        if (token.type == LT::RANGE)
        {
            parseToken(file, token);

            rng.high = strtoul(token.lexem.c_str(), nullptr, 10);

            parseToken(file, token);
        }

        res.push_back(rng);

    } while (token.type == LT::BAR);

    if (size)
    {
        if (token.type != LT::RIGHTPAREN)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected \")\" after SIZE", token.lexem);
            res.clear();
            return res;
        }

        parseToken(file, token);
    }

    if (token.type != LT::RIGHTPAREN)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected \")\"", token.lexem);
        res.clear();
    }

    return res;
}

IndexList Parser::parseIndexes(std::ifstream& file)
{
    Token token;
    Index indx;
    IndexList res;
    bool nextIsImplied = false;

    parseToken(file, token);

    if (token.type != LT::LEFTBRACKET)
    {
        return res;
    }

    parseToken(file, token);

    while (token.type != LT::RIGHTBRACKET && token.type != LT::ENDOFFILE)
    {
        if ((token.type == LT::LABEL) || (token.type & LT::SYNTAX_MASK))
        {
            indx.ilabel = token.lexem;
            indx.isimplied = nextIsImplied;

            res.push_back(indx);

            nextIsImplied = false;
        }
        else if (token.type == LT::IMPLIED)
        {
            nextIsImplied = true;
        }
        parseToken(file, token);
    }

    return res;
}

VarbindList Parser::parseVarbinds(std::ifstream& file)
{
    Token token;
    VarbindList res;

    parseToken(file, token);

    if (token.type != LT::LEFTBRACKET)
    {
        return res;
    }

    parseToken(file, token);

    while (token.type != LT::RIGHTBRACKET && token.type != LT::ENDOFFILE)
    {
        if (token.type == LT::LABEL || token.type & LT::SYNTAX_MASK)
        {
            res.push_back(token.lexem);
        }
        else if (token.type != LT::COMMA)
        {
            _errinf.isError = true;
            _errinf.description = formError("Expected \",\"", token.lexem);
            res.clear();
            break;
        }

        parseToken(file, token);
    }

    if (token.type != LT::RIGHTBRACKET)
    {
        _errinf.isError = true;
        _errinf.description = formError("Expected \"}\"", token.lexem);
    }

    return res;
}

bool Parser::checkUTC(const std::string& utc)
{
    size_t len;
    int year, month, day, hour, minute;

    len = utc.size();

    if (utc.back() != 'Z' && utc.back() != 'z')
    {
        _errinf.isError = true;
        _errinf.description = formError("Timestamp should end with Z", utc);
        return false;
    }

    if (len == 11)
    {
        len = sscanf(utc.c_str(), "%2d%2d%2d%2d%2dZ",
            &year, &month, &day, &hour, &minute);

        year += 1900;
    }
    else if (len == 13)
        len = sscanf(utc.c_str(), "%4d%2d%2d%2d%2dZ",
            &year, &month, &day, &hour, &minute);
    else
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad timestamp format (11 or 13 characters)", utc);
        return false;
    }

    if (len != 5)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad timestamp format", utc);
        return false;
    }

    if (month < 1 || month > 12)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad month in timestamp", utc);
        return false;
    }

    if (day < 1 || day > 31)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad day in timestamp", utc);
        return false;
    }

    if (hour < 0 || hour > 23)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad hour in timestamp", utc);
        return false;
    }

    if (minute < 0 || minute > 59)
    {
        _errinf.isError = true;
        _errinf.description = formError("Bad minute in timestamp", utc);
        return false;
    }

    return true;
}

void Parser::tossObjectIdentifier(std::ifstream& file, Token& token)
{
    int bracketCount = 1;
    token.type = LT::OTHER;
    parseToken(file, token);

    if (token.type != LT::LEFTBRACKET)
        return;

    while ((token.type != LT::RIGHTBRACKET || bracketCount > 0) && token.type != LT::ENDOFFILE)
    {
        parseToken(file, token);
        if (token.type == LT::LEFTBRACKET)
            bracketCount++;
        else if (token.type == LT::RIGHTBRACKET)
            bracketCount--;
    }

    if (token.type == LT::RIGHTBRACKET)
        token.type = LT::OBJID;
}

void Parser::eatSyntax(std::ifstream& file, Token& token)
{
    parseToken(file, token);
    LT lasttype = token.type;
    parseToken(file, token);

    switch (lasttype)
    {
    case LT::INTEGER:
    case LT::INTEGER32:
    case LT::UINTEGER32:
    case LT::UNSIGNED32:
    case LT::COUNTER:
    case LT::GAUGE:
    case LT::BITSTRING:
    case LT::LABEL:

        if (token.type == LT::LEFTBRACKET)
        {
            parseEnums(file);
            parseToken(file, token);
        }
        else if (token.type == LT::LEFTPAREN)
        {
            parseRanges(file);
            parseToken(file, token);
        }
        break;
    case LT::OCTETSTR:
    case LT::OPAQUE:
        if (token.type == LT::LEFTPAREN)
        {
            parseToken(file, token);
            if (token.type == LT::SIZE)
            {
                parseToken(file, token);
                if (token.type == LT::LEFTPAREN)
                {
                    parseRanges(file);
                    parseToken(file, token);

                    if (token.type == LT::RIGHTPAREN)
                    {
                        parseToken(file, token);
                        break;
                    }
                }
            }

            _errinf.isError = true;
            _errinf.description = formError("Bad SIZE syntax", token.lexem);
            return;
        }
        break;
    case LT::OBJID:
    case LT::NETADDR:
    case LT::IPADDR:
    case LT::TIMETICKS:
    case LT::NUL:
    case LT::NSAPADDRESS:
    case LT::COUNTER64:
        break;
    default:

        _errinf.isError = true;
        _errinf.description = formError("Bad SYNTAX", typeToStr(lasttype));
    }
}

bool Parser::complianceLookup(const std::string& name, const std::string& moduleName)
{

    if (moduleName.empty())
    {
        _objgroups.insert(name);
        return true;
    }

    return (_tree ? _tree->findNode(name, moduleName) : nullptr) != nullptr;
}

Parser::LoadStatus Parser::readModuleInternal(const std::string& moduleName)
{
    auto mp = _tree ? _tree->findModule(moduleName) : nullptr;

    if (mp)// && mp->moduleName == moduleName)
    {
        if (mp->isParsed)
            return LoadStatus::MODULE_ALREADY_LOADED;

        std::ifstream file(mp->fileName);

        if (!file.is_open())
            return LoadStatus::MODULE_LOAD_FAILED;

        size_t oldLine = _line;
        std::string oldModule = _moduleName;

        _line = 1;
        _moduleName = moduleName;

        parse(file);

        _line = oldLine;
        _moduleName = std::move(oldModule);

        file.close();

        if (_errinf.isError)
            return LoadStatus::MODULE_SYNTAX_ERROR;

        return LoadStatus::MODULE_LOADED_OK;

    }

    return LoadStatus::MODULE_NOT_FOUND;
}

Parser::LoadStatus Parser::readModuleReplacements(const std::string& oldModuleName)
{
    size_t size = _modCompats.size();
    LoadStatus current = LoadStatus::MODULE_NOT_FOUND;

    for (size_t i = 0; i < size; ++i)
    {
        auto const& mc = _modCompats[i];
        if (mc.oldModule == oldModuleName)
        {
            auto status = readModuleInternal(mc.newModule);

            if (status != LoadStatus::MODULE_LOADED_OK)
            {
                return status;
            }

            current = status;
        }
    }

    return current;
}

void Parser::scanObjlist(const NodeList& root, const Module::Ptr& mp, Objgroup& objlist)
{
    for (auto it = objlist.begin(); it != objlist.end(); )
    {
        bool found = false;

        for (auto const& node : root)
            if (node->label == *it)
            {
                it = objlist.erase(it);
                found = true;
                break;
            }

        if (!found && mp)
        {
            auto const& imports = mp->imports;
            size_t sizeImp = imports.size();

            for (size_t i = 0; i < sizeImp; ++i)
            {
                auto const& import = imports[i];
                size_t sizeLab = import.labels.size();

                for (size_t j = 0; j < sizeLab; ++j)
                    if (import.labels[j] == *it)
                    {
                        it = objlist.erase(it);
                        found = true;
                        break;
                    }

                if (found)
                    break;
            }
        }

        if (!found)
            it++;
    }


    if (!objlist.empty())
    {
        _errinf.isError = true;
        _errinf.description = "Unidentified objects in " + _moduleName + ":";

        for (auto const& obj : objlist)
            _errinf.description += " " + obj + ",";

        _errinf.description.pop_back();
    }
}

void Parser::resolveSyntax()
{

    for (auto it = _undefNodes.begin(); it != _undefNodes.end();)
    {
        auto const& syntax = it->syntax;
        size_t i = 0, TCsize = _tclist.size();

        for (; i < TCsize; ++i)
        {
            auto const& tc = _tclist[i];

            if (tc.descriptor == syntax)
            {
                auto& node = it->node;

                node->syntax = tc.syntax;
                node->hint = tc.hint;

                if (node->ranges.empty() && !tc.ragnes.empty())
                    node->ranges = tc.ragnes;

                if (node->enums.empty() && !tc.enums.empty())
                    node->enums = tc.enums;

                it = _undefNodes.erase(it);
                break;
            }
        }

        if (i == TCsize)
            ++it;
    }

    if (!_undefNodes.empty())
    {
        _errinf.isError = true;
        _errinf.description = "Unidentified objects in " + _moduleName + ":";

        for (auto const& undefNode : _undefNodes)
            _errinf.description += " " + undefNode.syntax + ",";

        _errinf.description.pop_back();
    }
}

std::string Parser::formError(const std::string& str, const std::string& lexem)
{
    return str +
        (!lexem.empty() ? " (" + lexem + ")" : "") +
        ": At line " + std::to_string(_line) + " in " +
        (_moduleName.empty() ? "(EMPTY)" : _moduleName);
}

void Parser::parseToken(std::ifstream& file, Token& token)
{
    char ch;
    enum { bdigits, xdigits, other } seenSymbols;
    bool more = true;

    while (more)
    {
        token.lexem.clear();
        more = false;

        do
        {
            ch = file.get();
            if (ch == '\n')
                ++_line;
        } while (isspace(ch) && ch != EOF);

        token.lexem += ch;

        switch (ch)
        {
        case EOF:
            token.type = LT::ENDOFFILE;
            return;
        case '"':
            token.lexem.pop_back();
            parseQuoteString(file, token);
            return;
        case '\'':                 /* binary or hex constant */
            seenSymbols = bdigits;
            while ((ch = file.get()) != EOF && ch != '\'')
            {
                switch (seenSymbols)
                {
                case bdigits:
                    if (ch == '0' || ch == '1')
                        break;
                    seenSymbols = xdigits;
                    /* FALL THROUGH */
                case xdigits:
                    if (isxdigit(ch))
                        break;
                    seenSymbols = other;
                case other:
                    break;
                }
                token.lexem += ch;
            }
            if (ch == '\'')
            {
                unsigned long   val = 0;
                ch = file.get();

                switch (ch)
                {
                case EOF:
                    token.type = LT::ENDOFFILE;
                    return;
                case 'b':
                case 'B':
                    if (seenSymbols > bdigits)
                    {
                        token.lexem += '\'';
                        token.type = LT::LABEL;
                        return;
                    }
                    for (int i = 1; i < token.lexem.size(); ++i)
                        val = val * 2 + token.lexem[i] - '0';

                    break;
                case 'h':
                case 'H':
                    if (seenSymbols > xdigits)
                    {
                        token.lexem += '\'';
                        token.type = LT::LABEL;
                        return;
                    }
                    for (int i = 1; i < token.lexem.size(); ++i)
                    {
                        ch = token.lexem[i];
                        if ('0' <= ch && ch <= '9')
                            val = val * 16 + ch - '0';
                        else if ('a' <= ch && ch <= 'f')
                            val = val * 16 + ch - 'a' + 10;
                        else if ('A' <= ch && ch <= 'F')
                            val = val * 16 + ch - 'A' + 10;
                    }
                    break;
                default:
                    token.lexem += '\'';
                    token.type = LT::LABEL;
                    return;
                }
                token.lexem = std::to_string(val);
                token.type = LT::NUMBER;
                return;
            }
            token.lexem += ch;
            token.type = LT::LABEL;
            return;

        case '(':
            token.type = LT::LEFTPAREN;
            return;
        case ')':
            token.type = LT::RIGHTPAREN;
            return;
        case '{':
            token.type = LT::LEFTBRACKET;
            return;
        case '}':
            token.type = LT::RIGHTBRACKET;
            return;
        case '[':
            token.type = LT::LEFTSQBRACK;
            return;
        case ']':
            token.type = LT::RIGHTSQBRACK;
            return;
        case ';':
            token.type = LT::SEMI;
            return;
        case ',':
            token.type = LT::COMMA;
            return;
        case '|':
            token.type = LT::BAR;
            return;
        case '.':
            token.lexem += file.get();

            if (token.lexem.back() == '.')
            {
                token.type = LT::RANGE;
                return;
            }
            file.unget();
            token.type = LT::LABEL;
            return;

        case ':':

            token.lexem += file.get();

            if (token.lexem.back() != ':')
            {
                file.unget();
                token.type = LT::LABEL;
                return;
            }

            token.lexem += file.get();
            if (token.lexem.back() != '=')
            {
                file.unget();
                token.type = LT::LABEL;
                return;
            }
            token.type = LT::EQUALS;
            return;

        case '-':
            ch = file.get();

            if (ch == '-')
            {
                token.lexem += ch;
                while ((ch != EOF) && (ch != '\n'))
                    ch = file.get();

                if (ch == '\n')
                    ++_line;

                more = true;
                continue;
            }

            file.unget();
            /* fallthrough */
        default:

            if (!isLabelChar(ch))
            {
                token.type = LT::LABEL;
                return;
            }

            while (true)
            {
                while (isLabelChar(ch = file.get()))
                    token.lexem += ch;


                file.unget();

                auto it = _lexemTable.find(token.lexem);

                if (it != _lexemTable.end())
                {
                    token.type = it->second;

                    if (token.type != LT::CONTINUE)
                        return;

                    while (isspace((ch = file.get())));

                    if (ch == '\n')
                        ++_line;

                    if (ch == EOF)
                    {
                        token.type = LT::ENDOFFILE;
                        return;
                    }

                    if (isalnum(ch))
                    {
                        token.lexem += ch;
                        continue;
                    }
                }

                break;
            }

            if (token.lexem[0] == '-' || isdigit((unsigned char)(token.lexem[0])))
            {
                for (int i = 1; i < token.lexem.size(); ++i)
                    if (!isdigit((unsigned char)token.lexem[i]))
                    {
                        token.type = LT::LABEL;
                        return;
                    }

                token.type = LT::NUMBER;
                return;
            }

            token.type = LT::LABEL;
        }
    }
}

std::string Parser::typeToStr(LT type)
{
    for (auto const& lexemPair : _lexemTable)
        if (lexemPair.second == type)
            return lexemPair.first;

    if (type == LT::OBJID)
        return "OBJECT IDENTIFIER";

    return "";
}

Strs Parser::getModuleReplacements(const std::string& oldModule)
{
    Strs res;

    for (size_t i = 0, size = _modCompats.size(); i < size; ++i)
    {
        auto const& mc = _modCompats[i];

        if (mc.oldModule == oldModule)
        {
            if (std::find(res.begin(), res.end(), mc.newModule) == res.end())
                res.push_back(mc.newModule);
        }
    }

    return res;
}

void Parser::parseQuoteString(std::ifstream& file, Token& token)
{
    //token.lexem.clear();
    bool newLine = false;
    char lastChar = '\0';

    for (char ch = file.get(); ch != EOF; ch = file.get())
    {
        if (ch == '\r')
            continue;

        if (ch == '\n')
        {
            newLine = true;
            ++_line;
        }
        else if (newLine && (ch == ' ' || ch == '\t'))
            continue;
        else
            newLine = false;

        if (ch == ' ' && lastChar == ' ')
            continue;

        if (ch == '"')
        {
            token.type = LT::QUOTESTRING;
            return;
        }

        token.lexem += ch;
        lastChar = ch;
    }

    token.type = LT::ENDOFFILE;
}

NodeList Parser::parse(std::ifstream& file)
{
    _line = 1;
    _moduleName.clear();
    _errinf.isError = false;
    _errinf.description.clear();

    Token token;
    NodeList result;
    NodeList nodes;
    Module::Ptr mp;
    std::string name;
    LT lasttype = LT::LABEL;

#define BETWEEN_MIBS          1
#define IN_MIB                2
    int state = BETWEEN_MIBS;

    while (token.type != LT::ENDOFFILE)
    {
        if (lasttype == LT::CONTINUE)
            lasttype = token.type;
        else
        {
            parseToken(file, token);
            lasttype = token.type;
        }

        switch (token.type)
        {
        case LT::END:
            if (state != IN_MIB)
            {
                _errinf.isError = true;
                _errinf.description = formError("END before BEGIN");
                return EmptyResult;
            }
            else
            {
                scanObjlist(result, mp, _objgroups);
                if (_errinf.isError)
                    return EmptyResult;

                scanObjlist(result, mp, _objects);
                if (_errinf.isError)
                    return EmptyResult;

                scanObjlist(result, mp, _notifs);
                if (_errinf.isError)
                    return EmptyResult;

                resolveSyntax();
                if (_errinf.isError)
                    return EmptyResult;

                if (_tree)
                    _tree->linkupNodes(result);

                if (!result.empty())
                {
                    _errinf.isError = true;
                    _errinf.description = "Can't link up nodes of " + _moduleName;
                    //return;
                }
            }

            state = BETWEEN_MIBS;
            continue;
        case LT::IMPORTS:

            parseImports(file);

            if (_errinf.isError)
                return EmptyResult;

            continue;
        case LT::EXPORTS:

            while (token.type != LT::SEMI && token.type != LT::ENDOFFILE)
                parseToken(file, token);

            continue;

        case LT::ENDOFFILE:

            scanObjlist(result, mp, _objgroups);
            if (_errinf.isError)
                continue;

            scanObjlist(result, mp, _objects);
            if (_errinf.isError)
                continue;

            scanObjlist(result, mp, _notifs);
            if (_errinf.isError)
                continue;

            resolveSyntax();

            continue;
        default:
            break;
        }

        name = token.lexem;
        parseToken(file, token);

        if (lasttype == LT::LABEL && token.type == LT::LEFTBRACKET)
        {
            while (token.type != LT::RIGHTBRACKET && token.type != LT::ENDOFFILE)
                parseToken(file, token);

            if (token.type == LT::ENDOFFILE)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected \"}\"", token.lexem);
                return EmptyResult;
            }

            parseToken(file, token);
        }

        switch (token.type)
        {
        case LT::DEFINITIONS:

            if (state != BETWEEN_MIBS)
            {
                _errinf.isError = true;
                _errinf.description = formError("Error, nested MIBS", token.lexem);
                return EmptyResult;
            }
            state = IN_MIB;

            while (token.type != LT::ENDOFFILE)
            {
                parseToken(file, token);
                if (token.type == LT::BEGIN)
                    break;
            }

            _moduleName = name;


            mp = _tree ? _tree->findModule(_moduleName) : nullptr;

            if (!mp)
            {
                _errinf.isError = true;
                _errinf.description = "Module " + _moduleName + " not found.";
                return EmptyResult;
            }

            if (mp->isParsed)
                return EmptyResult;

            mp->isParsed = true;

            break;
        case LT::OBJTYPE:
        {
            nodes = parseObjType(file, name);
            break;
        }
        case LT::OBJGROUP:
        {
            nodes = parseObjectGroup(file, name, LT::OBJECTS, _objects);
            break;
        }
        case LT::NOTIFGROUP:
        {
            nodes = parseObjectGroup(file, name, LT::NOTIFICATIONS, _notifs);
            break;
        }
        case LT::TRAPTYPE:
        {
            nodes = parseTrap(file, name);
            break;
        }
        case LT::NOTIFTYPE:
        {
            nodes = parseNotifType(file, name);
            break;
        }
        case LT::COMPLIANCE:
        {
            nodes = parseCompliance(file, name);
            break;
        }
        case LT::AGENTCAP:
        {
            nodes = parseCapabilities(file, name);
            break;
        }
        case LT::MACRO:
        {
            parseMacro(file).clear();
            break;
        }
        case LT::MODULEIDENTITY:
        {
            nodes = parseModuleIdentity(file, name);
            break;
        }
        case LT::OBJIDENTITY:
        {
            nodes = parseObjectGroup(file, name, LT::OBJECTS, _objects);
            break;
        }
        case LT::OBJECT:
        {
            parseToken(file, token);
            if (token.type != LT::IDENTIFIER)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected IDENTIFIER", token.lexem);
                return EmptyResult;
            }

            parseToken(file, token);
            if (token.type != LT::EQUALS)
            {
                _errinf.isError = true;
                _errinf.description = formError("Expected \"::=\"", token.lexem);
                return EmptyResult;
            }
            nodes = parseObjectid(file, name);
            break;
        }
        case LT::EQUALS:
        {
            nodes = parseASN(file, token, name);
            lasttype = LT::CONTINUE;
            break;
        }
        case LT::ENDOFFILE:
            break;
        default:
            _errinf.isError = true;
            _errinf.description = formError("Bad operator", token.lexem);
            return EmptyResult;
        }

        if (_errinf.isError)
            return EmptyResult;

        if (!nodes.empty())
        {
            auto& np = nodes.back();
            if (np->type == LT::NA)
                np->type = token.type;

            result.splice(result.end(), std::move(nodes));
        }

    }

    return result;
}

ErrorInfo Parser::lastErrorInfo()
{
    return _errinf;
}

Parser::TCList Parser::tcList()
{
    return _tclist;
}

Parser::UndefinedNode::UndefinedNode(const Node::Ptr& Node, const std::string& Syntax) :
    node(Node),
    syntax(Syntax)
{}
