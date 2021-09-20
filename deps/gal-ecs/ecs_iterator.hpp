#pragma once
#include "entity.hpp"
#include <memory>



class IdIterate
{
public:
    IdIterate(entity_t *pBegin, entity_t *pEnd)
        : pBegin(pBegin),
          pEnd(pEnd)
    {

    }
    class iterator


    {
    public:
        iterator(entity_t *pPtr)
            : pPtr(pPtr)
        {
        }

        iterator &operator++()
        {
            pPtr++;
            return *this;
        }

        iterator operator++(int)
        {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        entity_t &operator*() const { return *pPtr; }
        entity_t *operator->() { return pPtr; }

        friend bool operator==(const iterator &a, const iterator &b)
        {
            return a.pPtr == b.pPtr;
        }

        friend bool operator!=(const iterator &a, const iterator &b)
        {
            return a.pPtr != b.pPtr;
        }

    private:
        entity_t *pPtr;
    };

    class const_iterator
    {
    public:
        const_iterator(entity_t *pPtr)
            : pPtr(pPtr)
        {
        }

        const_iterator &operator++()
        {
            pPtr++;
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }

        entity_t &operator*() const { return *pPtr; }
        entity_t *operator->() { return pPtr; }

        friend bool operator==(const const_iterator &a, const const_iterator &b)
        {
            return a.pPtr == b.pPtr;
        }

        friend bool operator!=(const const_iterator &a, const const_iterator &b)
        {
            return a.pPtr != b.pPtr;
        }

    private:
        entity_t *pPtr;
    };

    iterator begin()
    {
        return iterator(pBegin);
    }

    iterator end()
    {
        return iterator(pEnd);
    }

    const_iterator begin() const
    {
        return const_iterator(pBegin);
    }

    const_iterator end() const
    {
        return const_iterator(pEnd);
    }

    size_t size() const
    {
        return pEnd - pBegin;
    }

private:

    entity_t* pBegin;
    entity_t* pEnd;
};

template<typename Component>
class ComponentIterate
{
public:
    ComponentIterate(Component* pBegin, Component* pEnd)
    :pBegin(pBegin),
    pEnd(pEnd)
    {
    }

    class iterator

    {
    public:
        iterator(Component *pPtr)
            : pPtr(pPtr)
        {
        }

        iterator &operator++()
        {
            pPtr++;
            return *this;
        }

        iterator operator++(int)
        {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        Component &operator*() const { return *pPtr; }
        Component *operator->() { return pPtr; }

        friend bool operator==(const iterator &a, const iterator &b)
        {
            return a.pPtr == b.pPtr;
        }

        friend bool operator!=(const iterator &a, const iterator &b)
        {
            return a.pPtr != b.pPtr;
        }

    private:
        Component *pPtr;
    };

    class const_iterator
    {
    public:
        const_iterator(Component *pPtr)
            : pPtr(pPtr)
        {
        }

        const_iterator &operator++()
        {
            pPtr++;
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }

        Component &operator*() const { return *pPtr; }
        Component *operator->() { return pPtr; }

        friend bool operator==(const const_iterator &a, const const_iterator &b)
        {
            return a.pPtr == b.pPtr;
        }

        friend bool operator!=(const const_iterator &a, const const_iterator &b)
        {
            return a.pPtr != b.pPtr;
        }

    private:
        Component *pPtr;
    };


    iterator begin()
    {
        return iterator(pBegin);
    }

    iterator end()
    {
        return iterator(pEnd);
    }

    const_iterator begin() const
    {
        return const_iterator(pBegin);
    }

    const_iterator end() const
    {
        return const_iterator(pEnd);
    }

    size_t size() const
    {
        return pEnd - pBegin;
    }

private:
    Component* pBegin;
    Component* pEnd;
};
