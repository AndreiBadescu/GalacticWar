#pragma once
#include "entity.hpp"
#include <vector>
#include <cassert>

class sparse_set
{
public:

    sparse_set(size_t initialCount = 0)
    {
        sparse.reserve(initialCount);
        dense.reserve(initialCount);
    }

    bool find(entity_t entity) const
    {
        if(sparse[getEntityID(entity)] < dense.size())
            if(dense[sparse[getEntityID(entity)]] == entity)
            {
                return true;
            }

        return false;
    }

    unsigned int find_index(entity_t entity)
    {
        assert(getEntityID(entity) <= sparse.size());
        return sparse[getEntityID(entity)];
    }

    void emplace(entity_t entity)
    {
        dense.emplace_back(entity);
        sparse.resize(getEntityID(entity) + 1);
        sparse[getEntityID(entity)] = dense.size() - 1;
    }

    void remove(entity_t entity)
    {
        unsigned int toRemoveIndex = find_index(entity);

        if(toRemoveIndex == dense.size() - 1)
            dense.pop_back();
        else
        {
            dense[toRemoveIndex] = dense.back();
            sparse[getEntityID(dense.back())] = toRemoveIndex;
            dense.pop_back();
        }
    }

    auto begin()
    {
        return dense.begin();
    }

    auto end()
    {
        return dense.end();
    }

    auto begin() const
    {
        return dense.begin();
    }

    auto end() const
    {
        return dense.end();
    }

    size_t size() const
    {
        return dense.size();
    }

    entity_t* data()
    {
        return dense.data();
    }

    const entity_t* data() const
    {
        return dense.data();
    }

private:

    std::vector<unsigned int> sparse;
    std::vector<entity_t> dense;
};