#pragma once
#include <cstddef>
#include <string>

class ItemStackBase
{
public:
    void *vtable;    // vtabe ptr
    void *mItem;     // offset 0x08
    void *mUserData; // offset 0x16
    void *mBlock;    // offset 0x24

    // a helper of some sort goes here
};