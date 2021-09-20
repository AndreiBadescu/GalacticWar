#pragma once
#include "RendererCommon.hpp"
#include "Utility/Math.hpp"

namespace gal
{

	enum class Tile : char {
		SAND,	 // 0
		WATER,   // 1
		LAVA,    // 2
		ICE,     // 3
		GRASS,   // 4
		SNOW,	 // 5
		MAX_TILE // 6 - doar pentru enum
	};

	enum class TileObst : char 	{
		FIRE,		// 0
		ROCK,		// 1
		TREE,		// 2
		ICESTONE,	// 3
		TOWER_OFF,	// 4
		TOWER_ALLY,	// 5
		TOWER_ENEMY,// 6
		MAX_OBST	// 7 - doar pentru enum
	};

	enum class UnitType
	{
		INFANTRY,
		ARTILLERY,
		SHIELD
	};
	inline constexpr u32 OBSTACLE_MASK_INFANTRY = 
		bit((u64)TileObst::ROCK + (u64)Tile::MAX_TILE)|
		bit((u64)Tile::WATER)|
		bit((u64)Tile::LAVA )|
		bit((u64)TileObst::TREE + (u64)Tile::MAX_TILE);

	inline constexpr u32 OBSTACLE_MASK_ARTILLERY =
		bit((u64)TileObst::ROCK + (u64)Tile::MAX_TILE)|
		bit((u64)Tile::WATER)|
		bit((u64)Tile::LAVA )|
		bit((u64)TileObst::FIRE + (u64)Tile::MAX_TILE)|
		bit((u64)TileObst::TREE + (u64)Tile::MAX_TILE);

	inline constexpr u32 OBSTACLE_MASK_SHIELD =
		bit((u64)TileObst::ROCK + (u64)Tile::MAX_TILE)|
		//bit((u64)Tile::WATER)|
		bit((u64)Tile::LAVA )|
		bit((u64)TileObst::TREE + (u64)Tile::MAX_TILE);

	struct Unit
	{
		u32 healthPoints;
		u32 actionPoints;
		u32 damagePoints;
		u32 attackRange;
		UnitType unitType;
		glm::uvec2 tile;	
	};

	struct Tower
	{
		glm::uvec2 tile;
		enum class Ownership
		{
			ALLY,
			ENEMY,
			NONE
		}ownership = Ownership::NONE;
		
	};

}