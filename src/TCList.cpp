#include "TCList.h"



TCList::TCList()
{}


TCList::~TCList()
{}

int TCList::getTCIndex(const std::string& descriptor)//, const std::string& module)
{
    size_t size = _tclist.size();

    for (size_t i = 0; i < size; ++i)
    {
        auto const& tc = _tclist[i];

        if (tc.descriptor == descriptor)// && (tc.module == module || module.empty()))
            return i;
    }

    return -1;
}

TC& TCList::getTC(int index)
{
    return _tclist[index];
}

void TCList::addTC(const TC& tc)
{
    _tclist.push_back(tc);
}
