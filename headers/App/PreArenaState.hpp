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

	enum class Stage
	{
		WAITING,
		TOWER_SELECTION,
		UNIT_PLACEMENT,
		FINISHED
	};

	class PreArenaState : public GameState {
	public:
		PreArenaState(bool isHost, const char* pIPAddress = nullptr);

		void Start() override;
		GameState* Input(Action* pAction) override;
		GameState* Update(Action* pAction) override;
		GameState* LateUpdate(Action* pAction) override { *pAction = Action::None; return nullptr; }
		void Render() override;

		
	public:
		std::mutex* pSocketMutex;
		std::condition_variable* pSocketCv;
		std::vector<Unit> yourUnits;
		std::vector<Unit> enemyUnits;
		std::vector<UI> unitSelectionUI;
		std::thread socketThread;

		Packet* pPacket;
		bool* pMainCanModify;
		bool* pExitThread;

		Tile tile_map[mapSize][mapSize];
		TileObst obst_map[mapSize][mapSize];
		Tower towers[MAX_TOWERS];
		Stage stage;
		UnitType selectedType;
		bool isHost;
		bool yourTurn;
	};
}