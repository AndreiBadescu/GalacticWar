#pragma once
#include "App/GameState.hpp"
#include "Utility/Math.hpp"
#include "Components.hpp"
#include "P2P/Packet.hpp"
#include "Renderer/UI.hpp"
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>


namespace gal {
	class PreArenaState;

	class ArenaState : public GameState {
	public:
		ArenaState() = default;
		ArenaState(PreArenaState* pPreArenaState);

		void Start() override;
		GameState* Input(GameState::Action* pAction) override;
		GameState* Update(GameState::Action* pAction) override;
		GameState* LateUpdate(GameState::Action* pAction) override { *pAction = GameState::Action::None; return nullptr; }
		void Render() override;
			
	public:
		std::mutex* pSocketMutex;
		std::condition_variable* pSocketCv;
		std::thread socketThread;
		Packet* pPacket;
		bool* pMainCanModify;
		bool isHost;
		bool yourTurn;

		Tile tile_map[mapSize][mapSize];
		TileObst obst_map[mapSize][mapSize];
		Tower towers[MAX_TOWERS];
		std::vector<Unit> yourUnits;
		std::vector<Unit> enemyUnits;
		std::vector<glm::uvec2> moveRangeTiles;
		std::vector<glm::uvec2> attackRangeTiles;
		glm::ivec2 m_camera;
		u32 selectedUnitIndex;
		bool unitIsEnemy;
		u32 money;
		UI endTurnButton;
		UI moneyUI;
		std::vector<UI> towerShopMenu;
		u32 uSelectedTowerIndex;
		UnitType towerUnitType;

	};
}
