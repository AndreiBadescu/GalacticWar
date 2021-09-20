#pragma once

using entity_t = unsigned int;
using entity_id_t = unsigned int;

namespace entity_traits
{
    constexpr unsigned int enabledMask = (1 << 31);
    constexpr unsigned int idMask = ~enabledMask;
}

inline bool isEntityEnabled(entity_t entity)
{
    return (entity & entity_traits::enabledMask) != 0;
}

inline entity_id_t getEntityID(entity_t entity)
{
    entity_id_t id = entity & entity_traits::idMask;
    return id;
}
