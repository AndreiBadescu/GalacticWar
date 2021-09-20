#pragma once
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <cassert>


class ecs_vector
{
private:
    using byte = uint8_t;

private:

public:

ecs_vector(size_t elementSize, size_t count = 0)
:pData((byte*)malloc(elementSize * count)),
pEnd(pData),
pCap(pData + elementSize * count),
elementSize(elementSize)
{

}

~ecs_vector()
{
    free(pData);
}

ecs_vector(const ecs_vector& other)
:pData((byte*)malloc(other.capacity() * other.elementSize)),
pEnd(pData + other.size() * other.elementSize),
pCap(pData + other.capacity() * other.elementSize),
elementSize(other.elementSize)
{
    memcpy(pData, other.pData, other.size() * other.elementSize);
}

ecs_vector(ecs_vector&& other) noexcept
:pData(other.pData),
pEnd(other.pEnd),
pCap(other.pCap),
elementSize(other.elementSize)
{
    other.pData = nullptr;
}

size_t size() const
{
    return (pEnd - pData) / elementSize;
}

size_t capacity() const
{
    return (pCap - pData) / elementSize;
}

byte* begin()
{
    return pData;
}

byte* end()
{
    return pEnd;
}

const byte* begin() const
{
    return pData;
}

const byte* end() const
{
    return pEnd;
}

byte& operator[](size_t index)
{
    return pData[index];
}

const byte& operator[](size_t index) const
{
    return pData[index];
}

template<typename Component>
Component* at(size_t index)
{
    assert(index < size() && "ecs_vector index is out of bounds");
    return (Component*)pData + index;
}

void reserve(size_t count)
{
    if (capacity() < count)
    {
        byte *pNewData = (byte *)realloc(pData, count * elementSize);
        if(pNewData)
        {

            pEnd  = pNewData + (pEnd - pData);
            pCap  = pNewData + (pCap - pData);
            pData = pNewData;
        } else
        {
            assert(false && "Not enough Memory");
        } 
    }
}

void resize(size_t newCount)
{
    reserve(newCount);
    pEnd = pData + newCount * elementSize;
}

void* emplace_back()
{
    if(pEnd + elementSize > pCap)
    {

        const size_t newCap = std::max<size_t>(size() + elementSize, capacity() * 2);
        reserve(newCap);
    }

    void* retval = pEnd;
    pEnd += elementSize;
    return retval;
}

void remove(size_t index)
{
    if(((pEnd - elementSize) - pData) / elementSize > index)
        memcpy(pData + index * elementSize, pEnd - elementSize, elementSize);
    pEnd -= elementSize;
}

void pop_back()
{
    assert(pEnd != pData && "Cannot pop_back an empty ecs_vector");
    pEnd -= elementSize;
}

public:
const size_t elementSize;

private:

    byte* pData;
    byte* pEnd;
    byte* pCap;
};