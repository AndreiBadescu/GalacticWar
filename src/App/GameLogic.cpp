#include "App/GameLogic.hpp"
#include "deps/perlin_noise/PerlinNoise.hpp"
#include "RendererCommon.hpp"
#include "deps/perlin_noise/PerlinNoise.hpp"
#include "Renderer/UI.hpp"
#include "App/Input.hpp"
#include "Utility/SoundManager.hpp"
#include <random>
#include <queue>
#include <iostream>
#include <cmath>
#include <stack>

#define COUNT_OF(arr) (sizeof(arr) / sizeof(arr[0]))


namespace gal {

	void perlinMap(float map[mapSize][mapSize], u32 seed, float frequency = 16, int octaves = 8) {
		siv::PerlinNoise perlin(seed);

		frequency = mapSize / frequency;
		for (int y = 0; y < mapSize; y++) {
			for (int x = 0; x < mapSize; x++) {
				map[y][x] = perlin.accumulatedOctaveNoise2D_0_1(x / frequency, y / frequency, octaves);
			}
		}
	}


	struct Range {
		f32 min;
		f32 max;
		Tile tileType;
	};

	struct Point {
		i32 y, x;
	};

	static const Point DIR[] = {
		{-1,0},
		{0,+1},
		{+1,0},
		{0,-1},
		{-1,-1},
		{-1,+1},
		{+1,+1},
		{+1,-1}
	};

	//SAND,	   // 0
	//WATER,   // 1
	//LAVA,    // 2
	//ICE,     // 3
	//GRASS,   // 4
	//SNOW,	   // 5
	//MAX_TILE // 6 - doar pentru enumerare

	static const Range ranges[] = {
		{0.0f, 0.2f, Tile::LAVA},	 // 0 - LAVA
		{0.2f, 0.35f, Tile::SAND},	 // 1 - SAND
		{0.35f, 0.5f, Tile::GRASS},  // 2 - GRASS
		{0.5f, 0.65f, Tile::WATER},	 // 3 - WATER
		{0.65f, 0.80f, Tile::ICE},	 // 4 - ICE
		{0.80f, 1.0f, Tile::SNOW}	 // 5 - SNOW
		//{0.6f, 0.65f},  // 5 - FIRE
		//{0.5f, 0.6f},   // 6 - ROCK
		//{0.65f, 1.0f},  // 7 - TREE
	};

	static constexpr int ZONE_OFFSET = mapSize / 15;
	static constexpr Range zones_rangeY[] = {
		{ZONE_OFFSET, mapSize / 2 - ZONE_OFFSET - 1},
		{mapSize / 2 + ZONE_OFFSET, mapSize - ZONE_OFFSET - 1}
		//{2,12},
		//{17,27}
	};

	static const Range zones_rangeX[] = {
		{ZONE_OFFSET, mapSize / 3 - ZONE_OFFSET - 1},
		{mapSize / 3 + ZONE_OFFSET, 2 * mapSize / 3 - ZONE_OFFSET - 1},
		{2 * mapSize / 3 + ZONE_OFFSET, mapSize - ZONE_OFFSET - 1}
		//{2,7},
		//{12,17},
		//{22,27}
	};

	constexpr u32 nr_of_ranges = COUNT_OF(ranges);
	constexpr u32 nr_of_zonesY = COUNT_OF(zones_rangeY);
	constexpr u32 nr_of_zonesX = COUNT_OF(zones_rangeX);

	internal_func inline bool IsInsideMap(const Point pos) {
		return pos.y >= 0 && pos.y < mapSize&&
			pos.x >= 0 && pos.x < mapSize;
	}

	internal_func inline u32 GetRandInRange(const u32 rmin, const u32 rmax) {
		if (rmin == 0) { return 1 + rand() % rmax; }
		return rmin + rand() % (rmax + 1 - rmin);
	}

	internal_func inline Tile AdvanceTile(Tile tile_map[mapSize][mapSize], Point origin_pos) {
		u8 cnt[(i8)Tile::MAX_TILE] = {};

		for (u32 i = 0; i < 8; ++i) {
			Point new_pos = {
				origin_pos.y + DIR[i].y,
				origin_pos.x + DIR[i].x
			};

			if (!IsInsideMap(new_pos)) {
				for (u32 j = 0; j < (i8)Tile::MAX_TILE; ++j) {
					++cnt[j];
				}
			}
			else {
				++cnt[(u8)tile_map[new_pos.y][new_pos.x]];
			}
		}

		u8 max_freq = cnt[0];
		Tile superior_type = (Tile)0;
		for (u32 i = 1; i < (i8)Tile::MAX_TILE; ++i) {
			if (cnt[i] > max_freq) {
				max_freq = cnt[i];
				superior_type = (Tile)i;
			}
		}

		if (max_freq > 4) {
			return superior_type;
		}
		return tile_map[origin_pos.y][origin_pos.x];
	}

	internal_func void RepairMap(Tile tile_map[mapSize][mapSize]) {
		Tile* tile_map_cpy = new Tile[mapTiles];
		memcpy(tile_map_cpy, tile_map, mapTiles * sizeof(tile_map[0][0]));
		for (i32 y = 0; y < mapSize; ++y) {
			for (i32 x = 0; x < mapSize; ++x) {
				tile_map_cpy[y * mapSize + x] = AdvanceTile(tile_map, { y,x });
			}
		}
		memcpy(tile_map, tile_map_cpy, mapTiles * sizeof(tile_map[0][0]));
		delete[] tile_map_cpy;
	}

	//SAND,	   // 0
	//WATER,   // 1
	//LAVA,    // 2
	//ICE,     // 3
	//FIRE,    // 4
	//ROCK,    // 5
	//TREE,    // 6
	//GRASS,   // 7
	//NOTHING, // 8

	internal_func void GenerateObstacles(const Tile tile_map[mapSize][mapSize],
		TileObst obst_map[mapSize][mapSize],
		const u32 seed) {
		memset(obst_map, (u8)TileObst::MAX_OBST, mapSize * mapSize * sizeof(obst_map[0][0]));
		std::mt19937 RandomEngine(seed);
		std::uniform_int_distribution<std::mt19937::result_type> Distribution(0, 99);
		for (int y = 0; y < mapSize; ++y) {
			for (int x = 0; x < mapSize; ++x) {
				if (tile_map[y][x] == Tile::SAND) {
					u32 chance = Distribution(RandomEngine);
					if (chance < 3) {
						obst_map[y][x] = TileObst::ROCK;

					}
					else if (chance < 7) {
						obst_map[y][x] = TileObst::FIRE;
					}
				}
				else if (tile_map[y][x] == Tile::GRASS) {
					u32 chance = Distribution(RandomEngine);
					if (chance < 3) {
						obst_map[y][x] = TileObst::ROCK;
					}
					else if (chance < 8) {
						obst_map[y][x] = TileObst::TREE;
					}
				}
				else if (tile_map[y][x] == Tile::ICE) {
					if (Distribution(RandomEngine) < 4) {
						obst_map[y][x] = TileObst::ICESTONE;
					}
				}
			}
		}
	}

	internal_func bool MapIsNotGood(const Tile tile_map[mapSize][mapSize]) {
		u32 bioms = (u32)Tile::MAX_TILE;
		bool* check = new bool[(u32)Tile::MAX_TILE];
		for (u32 i = 0; i < (u32)Tile::MAX_TILE; ++i) {
			check[i] = false;
		}
		for (u32 y = 0; y < mapSize; ++y) {
			for (u32 x = 0; x < mapSize; ++x) {
				if (!check[(u32)tile_map[y][x]]) {
					check[(u32)tile_map[y][x]] = true;
					--bioms;
					if (bioms == 0) {
						return false;
					}
				}
			}
		}
		return true;
	}

	internal_func void GenerateUnitRangeMap(
		glm::uvec2 pos, u32 ObstacleMask,
		int range, u8 rangeMap[mapSize][mapSize],
		const Tile tile_map[mapSize][mapSize],
		const TileObst obst_map[mapSize][mapSize],
		const std::vector<Unit>& yourUnits,
		const std::vector<Unit>& enemyUnits) {

		if (range < 0)
			return;
		rangeMap[pos.y][pos.x] = 1;
		for (int i = -1; i <= 1; i++)
		{
			for (int j = -1; j <= 1; j++)
			{
				if (abs(i) == abs(j)) {
					continue;
				}
				if (pos.y + i < mapSize && pos.x + j < mapSize)
					if (rangeMap[pos.y + i][pos.x + j] == 0)
					{
						bool noUnits = true;
						for (const Unit& unit : yourUnits)
						{
							if (unit.tile == glm::uvec2(pos.x + j, pos.y + i))
							{
								noUnits = false;
								break;
							}
						}
						if (!noUnits)
							continue;
						for (const Unit& unit : enemyUnits)
						{
							if (unit.tile == glm::uvec2(pos.x + j, pos.y + i))
							{
								noUnits = false;
								break;
							}
						}

						if (!noUnits)
							continue;

						if ((bit((u64)tile_map[pos.y + i][pos.x + j]) & ObstacleMask) == 0 &&
							((bit((u64)obst_map[pos.y + i][pos.x + j] + (u64)Tile::MAX_TILE) & ObstacleMask) == 0))
							GenerateUnitRangeMap({ pos.x + j,pos.y + i }, ObstacleMask, range - 1, rangeMap, tile_map, obst_map, yourUnits, enemyUnits);
					}
			}
		}
	}

	void GenerateMap(PreArenaState& as, u32 seed) {
		srand(seed);
		float map[mapSize][mapSize];
		perlinMap(map, seed, 2.0f, 4);
		for (u32 y = 0; y < mapSize; ++y) {
			for (u32 x = 0; x < mapSize; ++x) {
				for (u32 tex = 0; tex < nr_of_ranges; ++tex) {
					if (map[y][x] < ranges[tex].max) {
						as.tile_map[y][x] = ranges[tex].tileType;
						break;
					}
				}
			}
		}
		RepairMap(as.tile_map);
		memset(as.obst_map, (i32)TileObst::MAX_OBST, sizeof(as.obst_map));

		GenerateObstacles(as.tile_map, as.obst_map, seed);

		for (u32 i = 0; i < 2; ++i) {
			for (u32 j = 0; j < 3; ++j) {
				const u32 ind = i * 3 + j;
				as.towers[ind].tile = {
					GetRandInRange(zones_rangeX[j].min, zones_rangeX[j].max),
					GetRandInRange(zones_rangeY[i].min, zones_rangeY[i].max)
				};

				if (as.obst_map[as.towers[ind].tile.y][as.towers[ind].tile.x] != TileObst::MAX_OBST ||
					as.tile_map[as.towers[ind].tile.y][as.towers[ind].tile.x] == Tile::LAVA) {
					--j;
					continue;
				}

				bool bad_tower_place = true;
				for (u32 k = 0; k < 4; ++k) {
					Point new_pos = {
						as.towers[ind].tile.y + DIR[k].y,
						as.towers[ind].tile.x + DIR[k].x
					};
					if (as.obst_map[new_pos.y][new_pos.x] == TileObst::MAX_OBST) {
						bad_tower_place = false;
						break;
					}
				}
				if (bad_tower_place) {
					--j;
				}
				else {
					as.obst_map[as.towers[ind].tile.y][as.towers[ind].tile.x] = TileObst::TOWER_OFF;
				}
			}
		}

	}

	u32 GenerateMap(PreArenaState& as)
	{
		std::random_device r;
		u32 seed;
		do {
			seed = r();
			float map[mapSize][mapSize];
			perlinMap(map, seed, 2.0f, 4);
			for (u32 y = 0; y < mapSize; ++y) {
				for (u32 x = 0; x < mapSize; ++x) {
					for (u32 tex = 0; tex < nr_of_ranges; ++tex) {
						if (map[y][x] < ranges[tex].max) {
							as.tile_map[y][x] = ranges[tex].tileType;
							break;
						}
					}
				}
			}
			RepairMap(as.tile_map);
		} while (MapIsNotGood(as.tile_map));

		srand(seed);
		memset(as.obst_map, (i32)TileObst::MAX_OBST, sizeof(as.obst_map));

		GenerateObstacles(as.tile_map, as.obst_map, seed);

		for (u32 i = 0; i < 2; ++i) {
			for (u32 j = 0; j < 3; ++j) {
				const u32 ind = i * 3 + j;
				as.towers[ind].tile = {
					GetRandInRange(zones_rangeX[j].min, zones_rangeX[j].max),
					GetRandInRange(zones_rangeY[i].min, zones_rangeY[i].max)
				};

				if (as.obst_map[as.towers[ind].tile.y][as.towers[ind].tile.x] != TileObst::MAX_OBST ||
					as.tile_map[as.towers[ind].tile.y][as.towers[ind].tile.x] == Tile::LAVA) {
					--j;
					continue;
				}

				bool bad_tower_place = true;
				for (u32 k = 0; k < 4; ++k) {
					Point new_pos = {
						as.towers[ind].tile.y + DIR[k].y,
						as.towers[ind].tile.x + DIR[k].x
					};
					if (as.obst_map[new_pos.y][new_pos.x] == TileObst::MAX_OBST) {
						bad_tower_place = false;
						break;
					}
				}
				if (bad_tower_place) {
					--j;
				}
				else {
					as.obst_map[as.towers[ind].tile.y][as.towers[ind].tile.x] = TileObst::TOWER_OFF;
				}
			}
		}

		return seed;
	}

	void GenerateMoveRangeMap(ArenaState& as) {

		assert(as.selectedUnitIndex != 0 && "No Unit Selected");
		as.moveRangeTiles.clear();
		Unit& selectedUnit = as.yourUnits[as.selectedUnitIndex - 1];
		u8 rangeMap[mapSize][mapSize] = {};
		u32 obstacleMask = -1;
		switch (selectedUnit.unitType) {
		case UnitType::INFANTRY: {

			obstacleMask = OBSTACLE_MASK_INFANTRY;
			break;
		}
		case UnitType::ARTILLERY: {

			obstacleMask = OBSTACLE_MASK_ARTILLERY;
			break;
		}
		case UnitType::SHIELD: {

			obstacleMask = OBSTACLE_MASK_SHIELD;
			break;
		}
		}

		GenerateUnitRangeMap(
			selectedUnit.tile,

			obstacleMask,
			selectedUnit.actionPoints,
			rangeMap,
			as.tile_map,
			as.obst_map,
			as.yourUnits,
			as.enemyUnits);

		for (int i = 0; i < mapSize; i++) {
			for (int j = 0; j < mapSize; j++) {
				if (rangeMap[i][j] && selectedUnit.tile != glm::uvec2(j, i)) {
					as.moveRangeTiles.emplace_back(j, i);
				}
			}
		}
	}

	void GenerateAttackRangeMap(ArenaState& as) {
		assert(as.selectedUnitIndex != 0 && "No Unit selected");
		as.attackRangeTiles.clear();
		const Unit& un = as.yourUnits[as.selectedUnitIndex - 1];

		for (int y = -(i32)un.attackRange; y <= (i32)un.attackRange; y++)
			for (int x = -(i32)un.attackRange; x <= (i32)un.attackRange; x++) {
				if (tileDistance(glm::uvec2{ x,y } + un.tile, un.tile) <= un.attackRange)
				{
					as.attackRangeTiles.emplace_back(un.tile.x + x, un.tile.y + y);
				}
			}
	}

	void HandleLeftClick(ArenaState& as)
	{
		const glm::vec2 mousePos = Input::GetMousePos();
		glm::uvec2 hoveringTile = GetHoveringTile();

		if (!as.isHost)
		{
			hoveringTile.y = mapSize - hoveringTile.y - 1;
			hoveringTile.x = mapSize - hoveringTile.x - 1;
		}
		bool endTurn = false;

		const glm::vec2 endTurnPos = UItoNormalPos(as.endTurnButton.anchorPoint, as.endTurnButton.pos, as.endTurnButton.size);
		if (endTurnPos.x + as.endTurnButton.size.x >= mousePos.x && endTurnPos.x <= mousePos.x &&
			endTurnPos.y + as.endTurnButton.size.y >= mousePos.y && endTurnPos.y <= mousePos.y)
		{
			as.endTurnButton.buttonData.pButtonFunc(&endTurn);
		}

		if (endTurn == true)
		{
			SoundManager::Play("sounds/Effects/button_click.mp3");
			as.yourTurn = false; for (Unit& un : as.yourUnits)
			{
				switch (un.unitType)
				{
				case UnitType::INFANTRY: {
					un.actionPoints = UNIT_INFANTRY.actionPoints;
				}break;
				case UnitType::ARTILLERY: {
					un.actionPoints = UNIT_ARTILLERY.actionPoints;
				}break;
				case UnitType::SHIELD: {
					un.actionPoints = UNIT_SHIELD.actionPoints;
				}break;
				}
			}

			for (int i = 0; i < MAX_TOWERS; i++)
			{
				if (as.towers[i].ownership == Tower::Ownership::ALLY)
					as.money += 20;
			}

			as.pPacket->function_id = FunctionId::GIVE_TURN;
			*as.pMainCanModify = false;
			as.pSocketCv->notify_one();
			return;
		}

		u32 index = 1;
		bool didAnything = false;
		if (as.uSelectedTowerIndex)
		{
			for (UI& ui : as.towerShopMenu)
			{
				const glm::vec2 pos = UItoNormalPos(ui.anchorPoint, ui.pos, ui.size);
				if (pos.x + ui.size.x >= mousePos.x && pos.x <= mousePos.x &&
					pos.y + ui.size.y >= mousePos.y && pos.y <= mousePos.y &&
					ui.uiType == UI::Type::BUTTON)
				{
					ui.buttonData.pButtonFunc(&as.towerUnitType);
					didAnything = true;
					break;
				}
			}
		}
		else
		{
			as.selectedUnitIndex = 0;
			for (int i = 0; i < MAX_TOWERS; i++)
			{
				if (hoveringTile == as.towers[i].tile && as.towers[i].ownership == Tower::Ownership::ALLY)
				{
					as.uSelectedTowerIndex = i + 1;
					didAnything = true;
					break;
				}
			}
		}

		if (!didAnything)
		{
			as.uSelectedTowerIndex = 0;
			for (const Unit& un : as.yourUnits) {
				if (hoveringTile == un.tile) {
					as.selectedUnitIndex = index;
					as.unitIsEnemy = false;
					if (un.actionPoints != 0)
					{
						GenerateMoveRangeMap(as);
						GenerateAttackRangeMap(as);
					}
					else
					{
						as.moveRangeTiles.clear();
						as.attackRangeTiles.clear();
					}
					return;
				}
				index++;
			}

			as.selectedUnitIndex = 0;
			as.moveRangeTiles.clear();
			as.attackRangeTiles.clear();
			index = 1;
			for (const Unit& un : as.enemyUnits) {
				if (hoveringTile == un.tile) {
					as.selectedUnitIndex = index;
					as.unitIsEnemy = true;
					return;
				}
				index++;
			}
		}


	}

	void HandleRightClick(ArenaState& as) {
		const glm::vec2 mousePos = Input::GetMousePos();
		glm::uvec2 hoveringTile = GetHoveringTile();
		if (!as.isHost)
		{
			hoveringTile.x = mapSize - 1 - hoveringTile.x;
			hoveringTile.y = mapSize - 1 - hoveringTile.y;
		}

		if (hoveringTile.x >= mapSize || hoveringTile.y >= mapSize) {
			return;
		}

		if (as.uSelectedTowerIndex != 0 && as.money >= 50)
		{

			if (tileSquareDistance(as.towers[as.uSelectedTowerIndex - 1].tile, hoveringTile) <= 2)
			{
				as.money -= 50;
				u64 mask = 0;
				switch (as.towerUnitType)
				{
				case UnitType::INFANTRY: {
					mask = OBSTACLE_MASK_INFANTRY;
				}break;
				case UnitType::ARTILLERY: {
					mask = OBSTACLE_MASK_ARTILLERY;
				}break;
				case UnitType::SHIELD: {
					mask = OBSTACLE_MASK_SHIELD;
				}break;
				}
				if (
					(bit((u64)as.tile_map[hoveringTile.y][hoveringTile.x]) & mask) == 0 &&
					(bit((u64)as.obst_map[hoveringTile.y][hoveringTile.x]) & mask) == 0 ||
					as.obst_map[hoveringTile.y][hoveringTile.x] == TileObst::MAX_OBST
					) {

					SoundManager::Play("sounds/Effects/buy.mp3");

					as.yourUnits.emplace_back(GetUnitFromType(as.towerUnitType));
					as.yourUnits.back().tile = hoveringTile;

					size_t dataSize = 0;
					memset(as.pPacket, 0, sizeof(*as.pPacket));
					as.pPacket->function_id = FunctionId::BUY;
					memcpy(as.pPacket->data + dataSize, &as.yourUnits.back().unitType, sizeof(as.yourUnits.back().unitType));
					dataSize += sizeof(as.yourUnits.back().unitType);

					memcpy(as.pPacket->data + dataSize, &as.yourUnits.back().tile, sizeof(as.yourUnits.back().tile));
					dataSize += sizeof(as.yourUnits.back().tile);

					*as.pMainCanModify = false;
					as.pSocketCv->notify_one();


				}
			}

		}

		if (as.selectedUnitIndex != 0 && !as.unitIsEnemy) {
			Unit& selectedUnit = as.yourUnits[as.selectedUnitIndex - 1];
			for (glm::uvec2 tile : as.attackRangeTiles) {
				if (tile == hoveringTile) {
					for (int i = 0; i < MAX_TOWERS; i++)
					{
						if (tile == as.towers[i].tile)
						{
							as.towers[i].ownership = Tower::Ownership::ALLY;

							int8_t size = 0;
							as.pPacket->function_id = FunctionId::CAPTURE;

							memcpy(as.pPacket->data, &i, sizeof(i));
							size += sizeof(i);
							as.pPacket->size = size;
							*as.pMainCanModify = false;
							as.pSocketCv->notify_one();
							return;
						}
					}

					for (Unit& eUn : as.enemyUnits)
					{
						if (eUn.tile == tile)
						{
							SoundManager::Play("sounds/Effects/shoot_ally.mp3");

							int8_t size = 0;

							as.pPacket->function_id = FunctionId::ATTACK;

							memcpy(as.pPacket->data + size, &eUn.tile, sizeof(eUn.tile));
							size += sizeof(eUn.tile);

							memcpy(as.pPacket->data + size, &selectedUnit.damagePoints, sizeof(selectedUnit.damagePoints));
							size += sizeof(selectedUnit.damagePoints);

							as.pPacket->size = size;
							*as.pMainCanModify = false;
							as.pSocketCv->notify_one();

							eUn.healthPoints -= std::min(selectedUnit.damagePoints, eUn.healthPoints);
							selectedUnit.actionPoints = 0;
							as.attackRangeTiles.clear();
							as.moveRangeTiles.clear();
							break;
						}
					}
					break;
				}
			}

			for (glm::uvec2 tile : as.moveRangeTiles) {
				if (tile == hoveringTile) {

					int8_t size = 0;

					as.pPacket->function_id = FunctionId::MOVE;
					memcpy(as.pPacket->data + size, &selectedUnit.tile, sizeof(selectedUnit.tile));
					size += sizeof(selectedUnit.tile);
					memcpy(as.pPacket->data + size, &tile, sizeof(tile));
					size += sizeof(tile);
					as.pPacket->size = size;
					*as.pMainCanModify = false;
					as.pSocketCv->notify_one();
					selectedUnit.actionPoints -= tileDistance(hoveringTile, selectedUnit.tile);
					selectedUnit.tile = tile;

					if (selectedUnit.actionPoints != 0) {
						GenerateMoveRangeMap(as);
						GenerateAttackRangeMap(as);
					}
					else
					{
						as.selectedUnitIndex = 0;
						as.attackRangeTiles.clear();
						as.moveRangeTiles.clear();
					}
					return;
				}
			}

		}

		if (std::remove_if(as.enemyUnits.begin(), as.enemyUnits.end(), [](Unit& un) { return un.healthPoints == 0; }) != as.enemyUnits.end()) {
			SoundManager::Play("sounds/Effects/death.mp3");
			as.enemyUnits.pop_back();
		}
	}


	Unit GetUnitFromType(UnitType type)
	{
		switch (type)
		{
		case UnitType::INFANTRY: {
			return UNIT_INFANTRY;
		}break;

		case UnitType::ARTILLERY: {
			return UNIT_ARTILLERY;
		}break;

		case UnitType::SHIELD: {
			return UNIT_SHIELD;
		}break;
		}
	}
	void DecodeAttack(ArenaState& as, const Packet& pack)
	{
		glm::uvec2 tile;
		int damage;
		size_t size = 0;

		memcpy(&tile, pack.data + size, sizeof(tile));
		size += sizeof(tile);
		memcpy(&damage, pack.data + size, sizeof(damage));
		size += sizeof(damage);
		for (Unit& un : as.yourUnits)
		{
			if (un.tile == tile)
			{
				SoundManager::Play("sounds/Effects/shoot_enemy.mp3");
				un.healthPoints -= std::min<u32>(damage, un.healthPoints);
				break;
			}
		}
		if (std::remove_if(as.yourUnits.begin(), as.yourUnits.end(), [](Unit& un) { return (un.healthPoints == 0); }) != as.yourUnits.end()) {
			SoundManager::Play("sounds/Effects/death.mp3");
			as.yourUnits.pop_back();
		}

	}
	void DecodeMove(ArenaState& as, const Packet& pack)
	{
		glm::uvec2 start;
		glm::uvec2 dest;
		size_t size = 0;

		memcpy(&start, pack.data + size, sizeof(start));
		size += sizeof(start);
		memcpy(&dest, pack.data + size, sizeof(dest));
		size += sizeof(dest);

		for (Unit& un : as.enemyUnits)
		{
			if (un.tile == start)
			{
				un.tile = dest;
				return;
			}
		}

	}
	void DecodeBuy(ArenaState& as, const Packet& pack)
	{
		//unittype, tile
		UnitType type;
		glm::uvec2 tile;
		size_t size = 0;
		memcpy(&type, pack.data + size, sizeof(type));
		size += sizeof(type);
		memcpy(&tile, pack.data + size, sizeof(tile));
		size += sizeof(tile);
		switch (type)
		{
			case UnitType::INFANTRY: {
				as.enemyUnits.emplace_back(UNIT_INFANTRY);
			}break;
			case UnitType::ARTILLERY: {
				as.enemyUnits.emplace_back(UNIT_ARTILLERY);
			}break;
			case UnitType::SHIELD: {
				as.enemyUnits.emplace_back(UNIT_SHIELD);
			}break;
		}

		as.enemyUnits.back().tile = tile;
	}
	void DecodeCapture(ArenaState& as, const Packet& pack)
	{
		int towerIndex;
		memcpy(&towerIndex, pack.data, sizeof(towerIndex));
		as.towers[towerIndex].ownership = Tower::Ownership::ENEMY;
	}
}