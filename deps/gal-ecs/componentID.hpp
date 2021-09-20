#pragma once
#include <iostream>

inline unsigned int getID()
{
    static unsigned int ID = 0;
    return ID++;
}

template<typename Component>
unsigned int getComponentID()
{
    static unsigned int ID = getID();
    return ID;
}
