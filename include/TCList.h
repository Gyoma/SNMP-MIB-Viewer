#pragma once
#include "GlobalTypes.h"
#include <string>
#include <vector>


struct TC
{
    std::string		descriptor;
    LT				type;
    std::string     module;
    std::string		hint;

    EnumList		enums;
    RangeList		ragnes;

    //struct enum_list *enums;
    //struct range_list *ranges;
    std::string		description;
};

class TCList
{
public:
    TCList();
    ~TCList();

    int getTCIndex(const std::string& descriptor);// , const std::string& module = std::string());
    TC& getTC(int index);

    void addTC(const TC& tc);

public:

    std::vector<TC> _tclist;
};

