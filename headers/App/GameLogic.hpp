#pragma once
#include "Components.hpp"
#include "Utility/Math.hpp"
#include "ArenaState.hpp"
#include "PreArenaState.hpp"
#include <vector>

namespace gal {
	inline constexpr Unit UNIT_INFANTRY
	{
		50,
		2,
		50,
		1,
		UnitType::INFANTRY
	};

	inline constexpr Unit UNIT_ARTILLERY
	{
		100,
		1,
		75,
		3,
		UnitType::ARTILLERY
	};

	inline constexpr Unit UNIT_SHIELD
	{
		300, // hp
		2,   // ap
		0,  // damage
		0,	 // range
		UnitType::SHIELD
	};



	void GenerateMap(PreArenaState& as, u32 seed);
	u32  GenerateMap(PreArenaState& as);
	void GenerateRangeMap(ArenaState& as);
	void GenerateAttackRangeMap(ArenaState& as);
	void HandleLeftClick(ArenaState& as);
	void HandleRightClick(ArenaState& as);
	Unit GetUnitFromType(UnitType type);
	void DecodeAttack(ArenaState& as, const Packet& pack);
	void DecodeMove(ArenaState& as, const Packet& pack);
	void DecodeBuy(ArenaState& as, const Packet& pack);
	void DecodeCapture(ArenaState& as, const Packet& pack);
}