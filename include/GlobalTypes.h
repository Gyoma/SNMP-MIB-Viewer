#pragma once
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <vector>
#include <memory>
#include <string>


//Lexem types
enum LT
{
    //Keywords

    eCONTINUE = -1,
    eDEFINITIONS,
    eBEGIN,
    eEND,
    eNA,
    eENDOFFILE,
    eIMPORTS,
    eEXPORTS,
    eFROM,
    eEQUALS,
    eLEFTPAREN,
    eRIGHTPAREN,
    eLEFTBRACKET,
    eRIGHTBRACKET,
    eLEFTSQBRACK,
    eRIGHTSQBRACK,
    eSEMI,
    eCOMMA,
    eBAR,
    eRANGE,
    eLASTUPDATED,
    eORGANIZATION,
    eCONTACTINFO,
    eAUGMENTS,
    eUNITS,
    eREFERENCE,
    eNUM_ENTRIES,
    eOF,
    eSEQUENCE,
    eDESCRIPTION,
    eINDEX,
    eDEFVAL,
    eSIZE,
    eMAXACCESS,
    eACCESS,
    eMINACCESS,
    eSTATUS,
    eSYNTAX,
    eENTERPRISE,
    eDISPLAYHINT,
    eIMPLIED,
    eSUPPORTS,
    eINCLUDES,
    eVARIATION,
    eREVISION,
    eOBJECTS,
    eMODULE,
    ePRODREL,
    eWRSYNTAX,
    eCREATEREQ,
    eMANDATORYGROUPS,
    eGROUP,
    eCHOICE,
    eIMPLICIT,

    //Types

    eOTHER,
    eLABEL,
    eOBSOLETE,
    eOPTIONAL,
    eMODULEIDENTITY,
    eNOACCESS,
    eWRITEONLY,
    eNUMENTRIES,
    eNOTIFTYPE,
    eOBJGROUP,
    eCOMPLIANCE,
    eIDENTIFIER,
    eOBJECT,
    eREADWRITE,
    eREADCREATE,
    eREADONLY,
    eDEPRECATED,
    eMANDATORY,
    eCURRENT,
    eOBJTYPE,
    eTRAPTYPE,
    eACCNOTIFY,
    eCONVENTION,
    eNOTIFGROUP,
    eAGENTCAP,
    eMACRO,
    eNOTIMPL,
    eNOTIFICATIONS,
    eNUMBER,
    eQUOTESTRING,

    eSYNTAX_MASK = 0x80,
    eOBJID,
    eOCTETSTR,
    eINTEGER,
    eNETADDR,
    eIPADDR,
    eCOUNTER,
    eGAUGE,
    eTIMETICKS,
    eOPAQUE,
    eNULL,
    eBITSTRING,
    eNSAPADDRESS,
    eCOUNTER64,
    eUINTEGER32,
    eAPPSYNTAX,
    eOBJSYNTAX,
    eSIMPLESYNTAX,
    eOBJNAME,
    eNOTIFNAME,
    eVARIABLES,
    eUNSIGNED32,
    eINTEGER32,
    eOBJIDENTITY
};


using Strs = std::vector<std::string>;
using LexemTypeTable = std::unordered_map<std::string, LT>;
using NodeList = std::list<std::shared_ptr<struct Node>>;
using Objgroup = std::unordered_set<std::string>;

struct Token
{
    Token(size_t lexemCapacity = 128) :
        type(LT::eNA)
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
    int             value;
    std::string		label;
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
