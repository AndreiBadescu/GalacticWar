#pragma once
#include "cvec.hpp"
#include <deque>
#include <vector>
#include "ecs_iterator.hpp"

class entity_registry
{
public:
    entity_registry()
        : lastEntity(0)
    {
    }
    entity_registry(size_t cvecCount)
        : lastEntity(0)
    {
        componentVectors.reserve(cvecCount);
    }

    template <typename Component, typename... Args>
    Component &AddComponent(entity_t entity, Args &&...args)
    {
        assure<Component>();
        Component *pAt = (Component *)componentVectors[getComponentID<Component>()].emplace(entity);
        new (pAt) Component(std::forward<Args>(args)...);
        return *pAt;
    }

    template <typename Component>
    Component &GetComponent(entity_t entity)
    {
        assure<Component>();
        assert(componentVectors[getComponentID<Component>()].exists(entity));
        Component *pAt = (Component *)componentVectors[getComponentID<Component>()].find(entity);
        return *pAt;
    }

    template <typename Component>
    void RemoveComponent(entity_t entity)
    {
        assure<Component>();
        assert(componentVectors[getComponentID<Component>()].exists(entity));
        componentVectors[getComponentID<Component>()].remove(entity);
    }

    template <typename Component>
    IdIterate IterateComponentIDs()
    {
        assure<Component>();
        entity_t *pBegin = componentVectors[getComponentID<Component>()].sparse_begin();
        entity_t *pEnd = componentVectors[getComponentID<Component>()].sparse_end();

        return IdIterate(pBegin, pEnd);
    }

    template <typename Component>
    ComponentIterate<Component> IterateComponents()
    {
        assure<Component>();
        Component *pBegin = (Component *)componentVectors[getComponentID<Component>()].begin();
        Component *pEnd = (Component *)componentVectors[getComponentID<Component>()].end();

        return ComponentIterate<Component>(pBegin, pEnd);
    }

    void RemoveEntity(entity_t entity)
    {
        for (cvec &vec : componentVectors)
        {
            if (vec.exists(entity))
                vec.remove(entity);
        }
        if (entity == lastEntity - 1)
            lastEntity--;
        else
            removedEntities.push_back(entity);
    }

    entity_t CreateEntity()
    {
        entity_t retval;
        if (!removedEntities.empty())
        {
            retval = removedEntities.front();
            removedEntities.pop_front();
        }
        else
        {
            retval = lastEntity++;
            retval |= entity_traits::enabledMask;
        }
        return retval;
    }

    template<typename Component>
    bool HasComponent(entity_t e)
    {
        assure<Component>();
        return componentVectors[getComponentID<Component>()].exists(e);
    }

private:
    template <typename Component>
    void assure()
    {
        if (getComponentID<Component>() >= componentVectors.size())
        {
            componentVectors.emplace_back(sizeof(Component), 1);
        }
    }

private:
    std::vector<cvec> componentVectors;
    std::deque<entity_t> removedEntities;
    entity_t lastEntity;
};