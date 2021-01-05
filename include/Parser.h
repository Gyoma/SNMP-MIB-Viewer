#pragma once
#include "GlobalTypes.h"
#include "Module.h"
#include "TCList.h"
#include "Tree.h"
#include <string>
#include <fstream>
#include <memory>
#include <list>
#include <vector>

class Parser
{
	LexemTypeTable _lexemTypes = {
		{ "obsolete",				LT::eOBSOLETE},
		{ "Opaque",					LT::eOPAQUE},
		{ "optional" ,				LT::eOPTIONAL},
		{ "LAST-UPDATED",			LT::eLASTUPDATED },
		{ "ORGANIZATION",			LT::eORGANIZATION},
		{ "CONTACT-INFO",			LT::eCONTACTINFO},
		{ "MODULE-IDENTITY",		LT::eMODULEIDENTITY},
		{ "MODULE-COMPLIANCE",		LT::eCOMPLIANCE },
		{ "DEFINITIONS",			LT::eDEFINITIONS},
		{ "END",					LT::eEND},
		{ "AUGMENTS",				LT::eAUGMENTS},
		{ "not-accessible",			LT::eNOACCESS},
		{ "write-only",				LT::eWRITEONLY},
		{ "NsapAddress",			LT::eNSAPADDRESS},
		{ "UNITS",					LT::eUNITS},
		{ "REFERENCE",				LT::eREFERENCE},
		{ "NUM-ENTRIES" ,			LT::eNUMENTRIES},
		{ "BITSTRING",				LT::eBITSTRING},
		{ "BIT",					LT::eCONTINUE},
		{ "BITS",					LT::eBITSTRING},
		{ "Counter64",				LT::eCOUNTER64},
		{ "TimeTicks",				LT::eTIMETICKS},
		{ "NOTIFICATION-TYPE",		LT::eNOTIFTYPE},
		{ "OBJECT-GROUP" ,			LT::eOBJGROUP},
		{ "OBJECT-IDENTITY",		LT::eOBJIDENTITY },
		{ "IDENTIFIER",				LT::eIDENTIFIER},
		{ "OBJECT",					LT::eOBJECT},
		{ "NetworkAddress",			LT::eNETADDR},
		{ "Gauge",					LT::eGAUGE},
		{ "Gauge32",				LT::eGAUGE},
		{ "Unsigned32",				LT::eUNSIGNED32},
		{ "read-write",				LT::eREADWRITE},
		{ "read-create",			LT::eREADCREATE},
		{ "OCTETSTRING",			LT::eOCTETSTR},
		{ "OCTET",					LT::eCONTINUE},
		{ "OF",						LT::eOF},
		{ "SEQUENCE",				LT::eSEQUENCE},
		{ "NULL",					LT::eNULL},
		{ "IpAddress",				LT::eIPADDR},
		{ "UInteger32",				LT::eUINTEGER32},
		{ "INTEGER",				LT::eINTEGER},
		{ "Integer32",				LT::eINTEGER32},
		{ "Counter",				LT::eCOUNTER},
		{ "Counter32",				LT::eCOUNTER},
		{ "read-only",				LT::eREADONLY },
		{ "DESCRIPTION",			LT::eDESCRIPTION},
		{ "INDEX",					LT::eINDEX},
		{ "DEFVAL",					LT::eDEFVAL},
		{ "deprecated",				LT::eDEPRECATED},
		{ "SIZE",					LT::eSIZE},
		{ "MAX-ACCESS",				LT::eACCESS },
		{ "ACCESS",					LT::eACCESS},
		{ "mandatory",				LT::eMANDATORY},
		{ "current",				LT::eCURRENT},
		{ "STATUS",					LT::eSTATUS},
		{ "SYNTAX",					LT::eSYNTAX},
		{ "OBJECT-TYPE",			LT::eOBJTYPE},
		{ "TRAP-TYPE",				LT::eTRAPTYPE },
		{ "ENTERPRISE",				LT::eENTERPRISE},
		{ "BEGIN",					LT::eBEGIN},
		{ "IMPORTS",				LT::eIMPORTS},
		{ "EXPORTS",				LT::eEXPORTS},
		{ "accessible-for-notify",	LT::eACCNOTIFY},
		{ "TEXTUAL-CONVENTION",		LT::eCONVENTION},
		{ "NOTIFICATION-GROUP",		LT::eNOTIFGROUP },
		{ "DISPLAY-HINT",			LT::eDISPLAYHINT },
		{ "FROM",					LT::eFROM},
		{ "AGENT-CAPABILITIES",		LT::eAGENTCAP },
		{ "MACRO",					LT::eMACRO },
		{ "IMPLIED",				LT::eIMPLIED },
		{ "SUPPORTS",				LT::eSUPPORTS },
		{ "INCLUDES",				LT::eINCLUDES },
		{ "VARIATION",				LT::eVARIATION },
		{ "REVISION",				LT::eREVISION },
		{ "not-implemented",		LT::eNOTIMPL },
		{ "OBJECTS",				LT::eOBJECT },
		{ "NOTIFICATIONS",			LT::eNOTIFICATIONS },
		{ "MODULE",					LT::eMODULE },
		{ "MIN-ACCESS",				LT::eMINACCESS },
		{ "PRODUCT-RELEASE",		LT::ePRODREL },
		{ "WRITE-SYNTAX",			LT::eWRSYNTAX },
		{ "CREATION-REQUIRES",		LT::eCREATEREQ },
		{ "MANDATORY-GROUPS",		LT::eMANDATORYGROUPS },
		{ "GROUP",					LT::eGROUP },
		{ "CHOICE",					LT::eCHOICE },
		{ "IMPLICIT",				LT::eIMPLICIT },
		{ "ObjectSyntax",			LT::eOBJSYNTAX },
		{ "SimpleSyntax",			LT::eSIMPLESYNTAX },
		{ "ApplicationSyntax",		LT::eAPPSYNTAX },
		{ "ObjectName",				LT::eOBJNAME },
		{ "NotificationName",		LT::eNOTIFNAME },
		{ "VARIABLES",				LT::eVARIABLES }
	};


	struct ModuleCompatibility
	{
		std::string oldModule;
		std::string newModule;
		std::string tag;    /* empty string implies unconditional replacement,
							 * otherwise node identifier or prefix
							 */
		bool prefix;
	};

	std::vector<ModuleCompatibility> _modCompats = {

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
		{ "RFC1155-SMI", "SNMPv2-SMI", "", false},
		{ "RFC1213-MIB", "SNMPv2-SMI", "mib-2", false},
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

	enum LoadStatus
	{
		MODULE_NOT_FOUND,
		MODULE_LOADED_OK,
		MODULE_ALREADY_LOADED,
		MODULE_LOAD_FAILED,
		MODULE_SYNTAX_ERROR
	};

	//std::shared_ptr<ModuleList> _moduleList;
	TCList _tclist;
	Tree::Ptr _tree;

	Objgroup _objgroups, _objects, _notifs;
	ErrorInfo _errinf;

	size_t _line = 0;
	std::string _moduleName;

public:

	Parser(Tree::Ptr tree = nullptr);
	//Parser(std::shared_ptr<ModuleList> const& moduleList = NULL);
	~Parser();

	NodeList parse(std::ifstream& file);
	const ErrorInfo& lastErrorInfo();

public:

#define EmptyResult NodeList();


	void parseToken(std::ifstream& file, Token& token);
	void parseQuoteString(std::ifstream& file, Token& token);
	bool isLabelChar(char ch);
	void parseImports(std::ifstream& file, const std::string& moduleName);
	NodeList parseObjectGroup(std::ifstream& file, const std::string& objName, const std::string& moduleName, LT what, Objgroup& objgroup);
	NodeList parseObjType(std::ifstream& file, const std::string& objName, const std::string& moduleName);
	NodeList parseNotifType(std::ifstream& file, const std::string& objName, const std::string& moduleName);
	NodeList parseCompliance(std::ifstream& file, const std::string& objName, const std::string& moduleName);
	NodeList parseModuleIdentity(std::ifstream& file, const std::string& objName, const std::string& moduleName);
	NodeList parseCapabilities(std::ifstream& file, const std::string& objName, const std::string& moduleName);
	NodeList parseASN(std::ifstream& file, Token& token, const std::string& objName, const std::string& moduleName);
	NodeList parseTrap(std::ifstream& file, const std::string& objName, const std::string& moduleName);
	NodeList parseMacro(std::ifstream& file);

	NodeList mergeParsedObjectid(Node::Ptr& np, std::ifstream& file, const std::string& moduleName);
	NodeList parseObjectid(std::ifstream& file, const std::string& objName, const std::string& moduleName);
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
	LoadStatus readImportReplacements(const std::string& oldModuleName, Module::Imports& module);
	LoadStatus readModuleReplacements(const std::string& oldModuleName);

	void scanObjlist(const NodeList& root, const Module::Ptr& mp, Objgroup& list);

	std::string formError(const std::string& str, const std::string& lexem);

	//Error scan_objlist(const NodeList& list, std::vector<std::string>& objgroup);
};

