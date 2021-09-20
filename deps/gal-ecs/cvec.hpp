#pragma once
#include "sparse_set.hpp"
#include "componentID.hpp"
#include "ecs_vector.hpp"
#include <cassert>

class cvec
{
public:

    cvec(size_t elementSize, size_t initialCount = 0)
    :ss(initialCount),
    components(elementSize, initialCount)
    {
    }

    void* emplace(entity_t entity)
    {
        ss.emplace(entity);
        return components.emplace_back();
    }

    void remove(entity_t entity)
    {
        assert(ss.find(entity) && "cannot remove entity (not found)!");
        unsigned int index = ss.find_index(entity);
        components.remove(index);
        ss.remove(entity);
    }

    void resize(size_t count)
    {
        components.resize(count);
    }

    void reserve(size_t count)
    {
        components.reserve(count);
    }

    bool exists(entity_t entity) const
    {
        return ss.find(entity);
    }

    void* find(entity_t entity)
    {
        assert(ss.find(entity));
        return &components[ss.find_index(entity) * components.elementSize];
    }

    auto begin()
    {
        return components.begin();
    }

    auto end()
    {
        return components.end();
    }

    auto begin() const
    {
        return components.begin();
    }

    auto end() const
    {
        return components.end();
    }

    size_t size() const
    {
        return components.size();
    }

    size_t capacity() const
    {
        return components.capacity();
    }

    auto sparse_begin()
    {
        return ss.data();
    }

    auto sparse_end()
    {
        return ss.data() + ss.size();
    }

    auto sparse_begin() const
    {
        return ss.data();
    }

    auto sparse_end() const
    {
        return ss.data() + ss.size();
    }

private:
    sparse_set ss;
    ecs_vector components;
};
