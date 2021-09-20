#include "App/PreArenaState.hpp"
#include "App/Window.hpp"
#include "App/GameLogic.hpp"
#include "P2P/Client.hpp"
#include "P2P/Host.hpp"
#include "Renderer/BatchRenderer.hpp"
#include "App/Input.hpp"
#include "Utility/ErrorHandler.hpp"
#include "App/Components.hpp"
#include "Utility/SoundManager.hpp"
#include <random>

#ifdef _DEBUG
#define PR(x) std::cout<<#x<<'\n'; x
#else
#define PR(x) x
#endif

namespace gal {
	//seed e par => hostu alege si muta primu

	internal_func void HostFunc(Packet* pPacket, std::mutex* pMutex, std::condition_variable* pSocketCv, bool* pMainCanModify, bool* pExitThread) {
		std::unique_lock<std::mutex> lock(*pMutex);
		PreArenaState* pThis;
		memcpy(&pThis, pPacket->data, sizeof(pThis));
		u32 seed = GenerateMap(*pThis);
		if (!Host::StartSession()) {
			Host::EndSession();
			*pExitThread = true;
			pSocketCv->notify_one();
			return;
		}

		memcpy(pPacket->data, &seed, sizeof(seed));
		if (!Host::SendPacket(*pPacket))
		{
			*pExitThread = true;
			return;
		}
		////*pMainCanModify = true;
		pSocketCv->notify_one();
		////sa se genereze mapa	
		pSocketCv->wait(lock);

		if (seed % 2 == 0)
		{
			//pentru alegerea turnurilor
			pSocketCv->wait(lock);
			// se trimite coordonatele la turn
			if (!Host::SendPacket(*pPacket))
			{
				*pExitThread = false;
				return;
			}
			//primesc packet
			*pPacket = Host::ReceivePacket();
			*pMainCanModify = true;
			pSocketCv->wait(lock);
		}
		else
		{
			//turnurile
			*pPacket = Host::ReceivePacket();
			//alegi turnurile
			*pMainCanModify = true;
			pSocketCv->wait(lock);
			if (!Host::SendPacket(*pPacket))
			{
				*pExitThread = true;
				return;
			}
		}

		bool yourTurn = (seed % 2 == 0);

		while (Window::IsRunning()) {
			if (yourTurn) {
				*pMainCanModify = true;
				pSocketCv->wait(lock);
				if (!Host::SendPacket(*pPacket))
				{
					std::cout << "aideplm\n";
					*pExitThread = true;
					return;
				}
				if (pPacket->function_id == FunctionId::GIVE_TURN) {
					yourTurn = false;
				}
			}
			else {
				ZeroMemory(pPacket, sizeof(*pPacket));
				*pPacket = Host::ReceivePacket();

				if (pPacket->function_id == FunctionId::GIVE_TURN) {
					yourTurn = true;
				}
				*pMainCanModify = true;
				pSocketCv->wait(lock);
			}
		}
		Host::EndSession();
	}

	internal_func void ClientFunc(Packet* pPacket, std::mutex* pMutex, std::condition_variable* pSocketCv, bool* pMainCanModify, bool* pExitThread, const char* pIPAddress) {

		std::unique_lock<std::mutex> lock(*pMutex);

		if (!Client::StartSession(pIPAddress)) {
			Client::EndSession();

			pSocketCv->notify_one();
			return;
		}

		*pPacket = Client::ReceivePacket();
		//am primit seedul
		u32 seed;
		memcpy(&seed, pPacket->data, sizeof(seed));

		//*pMainCanModify = true;
		pSocketCv->notify_one();
		//am generat mapa
		pSocketCv->wait(lock);


		if (seed % 2 == 0) {
			*pPacket = Client::ReceivePacket();
			//oponentul a terminat de ales
			*pMainCanModify = true;
			//am ales turnurile etc.
			pSocketCv->wait(lock);
			Client::SendPacket(*pPacket);

		}
		else {
			pSocketCv->wait(lock);
			Client::SendPacket(*pPacket);
			*pPacket = Client::ReceivePacket();
			*pMainCanModify = true;
			pSocketCv->wait(lock);
		}

		bool yourTurn = seed % 2 == 1;

		while (Window::IsRunning()) {
			if (yourTurn) {
				*pMainCanModify = true;
				pSocketCv->wait(lock);
				if (!Client::SendPacket(*pPacket))
				{
					std::cout << "aideplm\n";
					*pExitThread = true;
					return;
				}
				if (pPacket->function_id == FunctionId::GIVE_TURN) {
					yourTurn = false;
				}
			}
			else {
				ZeroMemory(pPacket, sizeof(*pPacket));
				*pPacket = Client::ReceivePacket();
				if (pPacket->function_id == FunctionId::GIVE_TURN) {
					yourTurn = true;
				}
				*pMainCanModify = true;
				pSocketCv->wait(lock);
			}
		}
		Client::EndSession();
	}



	gal::PreArenaState::PreArenaState(bool isHost, const char* pIPAddress) :
		isHost(isHost) {
		

		pSocketMutex = new std::mutex;
		pPacket = new Packet;
		pSocketCv = new std::condition_variable;
		pMainCanModify = new bool(false);
		pExitThread = new bool(false);

		std::unique_lock<std::mutex> lock(*pSocketMutex);

		if (isHost) {
			PreArenaState* pThis = this;
			memcpy(pPacket->data, &pThis, sizeof(pThis));

			socketThread = std::thread(HostFunc, pPacket, pSocketMutex, pSocketCv, pMainCanModify, pExitThread);
			pSocketCv->wait(lock);
			if (pPacket->function_id == FunctionId::CORRUPTED_PACKET) {
				return;
			}
			else
			{
				u32 seed;
				memcpy(&seed, pPacket->data, sizeof(seed));
				yourTurn = (seed % 2 == 0);
				if (seed % 2 == 0)
				{
					stage = Stage::TOWER_SELECTION;
				}
				else
				{
					stage = Stage::WAITING;
				}
			}
			pSocketCv->notify_one();
			lock.unlock();

		}
		else {
			socketThread = std::thread(ClientFunc, pPacket, pSocketMutex, pSocketCv, pMainCanModify, pExitThread, pIPAddress);
			pSocketCv->wait(lock);
			if (pPacket->function_id == FunctionId::CORRUPTED_PACKET) {
				return;
			}
			u32 seed;
			memcpy(&seed, pPacket->data, sizeof(seed));
			GenerateMap(*this, seed);
			yourTurn = (seed % 2 == 1);
			if (seed % 2 == 1)
			{
				stage = Stage::TOWER_SELECTION;
			}
			else
			{
				stage = Stage::WAITING;
			}
			pSocketCv->notify_one();
			lock.unlock();
		}
	}


	void gal::PreArenaState::Start() {
		SoundManager::Stop("sounds/background.mp3");

		UI ui{};

		ui.anchorPoint = UI::Anchor::TM;
		ui.size = { 200,100 };
		ui.pos = { -200,0 };
		ui.textureID = TextureID::COLOR_GREEN;
		ui.uiType = UI::Type::BUTTON;
		ui.text = "Infantry";
		ui.buttonData.pButtonFunc = [](void* pData) {

			*(UnitType*)pData = UnitType::INFANTRY;
		};

		unitSelectionUI.emplace_back(ui);
		ui.pos = {};
		ui.text = "Artillery";
		ui.buttonData.pButtonFunc = [](void* pData) {

			*(UnitType*)pData = UnitType::ARTILLERY;
		};

		unitSelectionUI.emplace_back(ui);
		ui.pos = { 200,0 };
		ui.text = "Shield";
		ui.buttonData.pButtonFunc = [](void* pData) {

			*(UnitType*)pData = UnitType::SHIELD;
		};

		unitSelectionUI.emplace_back(ui);

	}

	GameState* PreArenaState::Input(Action* pAction)
	{
		*pAction = Action::None;
		glm::uvec2 hoveringTile = GetHoveringTile();
		const glm::vec2 mousePos = Input::GetMousePos();

		if (!isHost)
		{
			hoveringTile.x = mapSize - 1 - hoveringTile.x;
			hoveringTile.y = mapSize - 1 - hoveringTile.y;
		}

		if (Input::GetMouseButton(0) == KeyState::Release)
		{
			if (stage == Stage::TOWER_SELECTION)
			{
				bool check = false;
				if (isHost)
				{
					if (hoveringTile.y > 15)
						check = true;
				}
				else
				{
					if (hoveringTile.y < 15)
						check = true;
				}
				if (check)
					for (int i = 0; i < MAX_TOWERS; i++)
					{
						if (towers[i].tile == hoveringTile && towers[i].ownership == Tower::Ownership::NONE)
						{
							towers[i].ownership = Tower::Ownership::ALLY;
							stage = Stage::UNIT_PLACEMENT;
						}
					}
			}
			else if (stage == Stage::UNIT_PLACEMENT)
			{
				bool newUnit = false;
				for (UI& ui : unitSelectionUI)
				{
					glm::vec2 pos = UItoNormalPos(ui.anchorPoint, ui.pos, ui.size);
					if (pos.x + ui.size.x >= mousePos.x && pos.x <= mousePos.x &&
						pos.y + ui.size.y >= mousePos.y && pos.y <= mousePos.y &&
						ui.uiType == UI::Type::BUTTON)
					{
						ui.buttonData.pButtonFunc(&selectedType);
						newUnit = true;
						break;
					}
				}

				if (!newUnit)
				{
					int ownedTowerIndex = 0;
					for (int i = 0; i < MAX_TOWERS; i++)
						if (towers[i].ownership == Tower::Ownership::ALLY)
							ownedTowerIndex = i;

					if (tileSquareDistance(towers[ownedTowerIndex].tile, hoveringTile) <= 2)
					{
						u64 mask = 0;
						switch (selectedType)
						{
						case UnitType::INFANTRY: {
							mask = OBSTACLE_MASK_INFANTRY;
						}break;
						case UnitType::ARTILLERY: {
							mask = OBSTACLE_MASK_INFANTRY;
						}break;
						case UnitType::SHIELD: {
							mask = OBSTACLE_MASK_INFANTRY;
						}break;
						}
						if (
							(bit((u64)tile_map[hoveringTile.y][hoveringTile.x]) & mask) == 0 &&
							(bit((u64)obst_map[hoveringTile.y][hoveringTile.x]) & mask) == 0 ||
							obst_map[hoveringTile.y][hoveringTile.x] == TileObst::MAX_OBST
							) {
								{
									yourUnits.emplace_back(GetUnitFromType(selectedType));
									yourUnits.back().tile = hoveringTile;
								}
								if (yourUnits.size() == 3)
								{
									size_t dataSize = 0;
									ZeroMemory(pPacket, sizeof(*pPacket));

									memcpy(pPacket->data + dataSize, &ownedTowerIndex, sizeof(ownedTowerIndex));
									dataSize += sizeof(ownedTowerIndex);

									for (Unit& un : yourUnits)
									{
										memcpy(pPacket->data + dataSize, &un.unitType, sizeof(un.unitType));
										dataSize += sizeof(un.unitType);

										memcpy(pPacket->data + dataSize, &un.tile, sizeof(un.tile));
										dataSize += sizeof(un.tile);

									}
									pSocketCv->notify_one();
									stage = Stage::FINISHED;
								}
						}
					}
				}

			}
		}
		return nullptr;
	}

	GameState* PreArenaState::Update(Action* pAction)
	{
		if (enemyUnits.size() == 3 && yourUnits.size() == 3)
		{
			if (yourTurn)
				pSocketCv->notify_one();
			*pAction = Action::Switch;
			return new ArenaState(this);
		}
		if (*pMainCanModify)
		{
			size_t dataSize = 0;
			int enemyTowerIndex;
			memcpy(&enemyTowerIndex, pPacket->data + dataSize, sizeof(enemyTowerIndex));
			dataSize += sizeof(enemyTowerIndex);
			for (int i = 0; i < 3; i++)
			{
				UnitType uType;
				memcpy(&uType, pPacket->data + dataSize, sizeof(uType));
				dataSize += sizeof(uType);

				enemyUnits.emplace_back(GetUnitFromType(uType));
				glm::uvec2 tile;
				memcpy(&tile, pPacket->data + dataSize, sizeof(glm::uvec2));
				enemyUnits.back().tile = tile;
				dataSize += sizeof(glm::uvec2);
			}

			towers[enemyTowerIndex].ownership = Tower::Ownership::ENEMY;
			*pMainCanModify = false;
			if (stage == Stage::WAITING)
				stage = Stage::TOWER_SELECTION;
		}

		return nullptr;
	}

	void gal::PreArenaState::Render() {

		glm::uvec2 hoveringTile = GetHoveringTile();

		BatchRenderer::BeginScene();
		if (!isHost)
		{
			hoveringTile.x = mapSize - 1 - hoveringTile.x;
			hoveringTile.y = mapSize - 1 - hoveringTile.y;


			for (int i = 0; i < mapSize; i++) {
				for (int j = 0; j < mapSize; j++) {
					if (hoveringTile.x == j && hoveringTile.y == i)
						BatchRenderer::DrawTile({ mapSize - 1 - j, mapSize - 1 - i }, tileSize * 1.2f, (u32)tile_map[i][j]);
					else
						BatchRenderer::DrawTile({ mapSize - 1 - j, mapSize - 1 - i }, tileSize, (u32)tile_map[i][j]);
				}
			}

			for (Unit& un : yourUnits)
			{
				TextureID textureID;
				switch (un.unitType)
				{
					case UnitType::INFANTRY: {
						textureID = TextureID::INFANTRY;
						break;
					}
					case UnitType::ARTILLERY: {
						textureID = TextureID::ARTILLERY;
						break;
					}
					case UnitType::SHIELD: {
						textureID = TextureID::SHIELD;
						break;
					}
					default:
					{
						LOG_FATAL("CORRUPTED yourUnits vector");
					}
				}
				BatchRenderer::DrawTile(glm::uvec2{ mapSize - 1, mapSize - 1 } - un.tile, { tileSize.x, tileSize.y * 1.5f }, textureID);
			}

			for (Unit& un : enemyUnits)
			{
				TextureID textureID;
				switch (un.unitType)
				{
					case UnitType::INFANTRY: {
						textureID = TextureID::INFANTRY;
						break;
					}
					case UnitType::ARTILLERY: {
						textureID = TextureID::ARTILLERY;
						break;
					}
					case UnitType::SHIELD: {
						textureID = TextureID::SHIELD;
						break;
					}
					default:
					{
						LOG_FATAL("CORRUPTED enemyUnits vector");
					}
				}
				BatchRenderer::DrawTile(glm::uvec2{ mapSize - 1, mapSize - 1 } - un.tile, { tileSize.x, tileSize.y * 1.5f }, textureID);
			}



			for (u32 y = 0; y < mapSize; ++y) {
				for (u32 x = 0; x < mapSize; ++x) {
					if (obst_map[y][x] == TileObst::MAX_OBST) { continue; }

					if (obst_map[y][x] == TileObst::TOWER_OFF ||
						obst_map[y][x] == TileObst::TOWER_ALLY ||
						obst_map[y][x] == TileObst::TOWER_ENEMY)
						continue;
					else if(obst_map[y][x] == TileObst::TREE)
						BatchRenderer::DrawTile(glm::uvec2{ mapSize - 1, mapSize - 1 } - glm::uvec2{ x,y }, { tileSize.x, tileSize.y * 2.0f }, (u32)Tile::MAX_TILE + (u32)obst_map[y][x]);
					else
						BatchRenderer::DrawTile(glm::uvec2{ mapSize - 1, mapSize - 1 } - glm::uvec2{ x,y }, tileSize, (u32)Tile::MAX_TILE + (u32)obst_map[y][x]);
				}
			}


			for (int i = 0; i < MAX_TOWERS; i++)
			{
				if (hoveringTile.y < 15 && stage == Stage::TOWER_SELECTION && towers[i].tile == hoveringTile && towers[i].ownership == Tower::Ownership::NONE)
					BatchRenderer::DrawTile(glm::uvec2{ mapSize - 1, mapSize - 1 } - towers[i].tile, { tileSize.x,tileSize.y * 2.2f }, TextureID::TOWER_HOVER);
				else if (towers[i].ownership == Tower::Ownership::NONE)
					BatchRenderer::DrawTile(glm::uvec2{ mapSize - 1, mapSize - 1 } - towers[i].tile, { tileSize.x,tileSize.y * 2.2f }, TextureID::TOWER_OFF);
				else if (towers[i].ownership == Tower::Ownership::ALLY)
					BatchRenderer::DrawTile(glm::uvec2{ mapSize - 1, mapSize - 1 } - towers[i].tile, { tileSize.x,tileSize.y * 2.2f }, TextureID::TOWER_ALLY);
				else if (towers[i].ownership == Tower::Ownership::ENEMY)
					BatchRenderer::DrawTile(glm::uvec2{ mapSize - 1, mapSize - 1 } - towers[i].tile, { tileSize.x,tileSize.y * 2.2f }, TextureID::TOWER_ENEMY);
			}
		}
		else
		{
			for (int i = 0; i < mapSize; i++) {
				for (int j = 0; j < mapSize; j++) {
					if (hoveringTile.x == j && hoveringTile.y == i)
						BatchRenderer::DrawTile(glm::uvec2{ j, i }, tileSize * 1.2f, (u32)tile_map[i][j]);
					else
						BatchRenderer::DrawTile(glm::uvec2{ j, i }, tileSize, (u32)tile_map[i][j]);
				}
			}

			for (Unit& un : yourUnits)
			{
				TextureID textureID;
				switch (un.unitType)
				{
				case UnitType::INFANTRY: {
					textureID = TextureID::INFANTRY;
					break;
				}
				case UnitType::ARTILLERY: {
					textureID = TextureID::ARTILLERY;
					break;
				}
				case UnitType::SHIELD: {
					textureID = TextureID::SHIELD;
					break;
				}
				default:
				{
					LOG_FATAL("CORRUPTED yourUnits vector");
				}
				}
				BatchRenderer::DrawTile(un.tile, { tileSize.x, tileSize.y * 1.5f }, textureID);
			}

			for (Unit& un : enemyUnits)
			{
				TextureID textureID;
				switch (un.unitType)
				{
				case UnitType::INFANTRY: {
					textureID = TextureID::INFANTRY;
					break;
				}
				case UnitType::ARTILLERY: {
					textureID = TextureID::ARTILLERY;
					break;
				}
				case UnitType::SHIELD: {
					textureID = TextureID::SHIELD;
					break;
				}
				default:
				{
					LOG_FATAL("CORRUPTED enemyUnits vector");
				}
				}
				BatchRenderer::DrawTile(un.tile, { tileSize.x, tileSize.y * 1.5f }, textureID);
			}

			for (u32 y = 0; y < mapSize; ++y) {
				for (u32 x = 0; x < mapSize; ++x) {
					if (obst_map[y][x] == TileObst::MAX_OBST) { continue; }

					if (obst_map[y][x] == TileObst::TOWER_OFF ||
						obst_map[y][x] == TileObst::TOWER_ALLY ||
						obst_map[y][x] == TileObst::TOWER_ENEMY)
						continue;
					else if(obst_map[y][x] == TileObst::TREE)
						BatchRenderer::DrawTile(glm::uvec2{ x,y }, { tileSize.x, tileSize.y * 2.0f }, (u32)Tile::MAX_TILE + (u32)obst_map[y][x]);
					else
						BatchRenderer::DrawTile(glm::uvec2{ x,y }, tileSize, (u32)Tile::MAX_TILE + (u32)obst_map[y][x]);
				}
			}


			for (int i = 0; i < MAX_TOWERS; i++)
			{
				if (hoveringTile.y > 15 && stage == Stage::TOWER_SELECTION && towers[i].tile == hoveringTile && towers[i].ownership == Tower::Ownership::NONE)
					BatchRenderer::DrawTile(towers[i].tile, { tileSize.x,tileSize.y * 2.2f }, TextureID::TOWER_HOVER);
				else if (towers[i].ownership == Tower::Ownership::NONE)
					BatchRenderer::DrawTile(towers[i].tile, { tileSize.x,tileSize.y * 2.2f }, TextureID::TOWER_OFF);
				else if (towers[i].ownership == Tower::Ownership::ALLY)
					BatchRenderer::DrawTile(towers[i].tile, { tileSize.x,tileSize.y * 2.2f }, TextureID::TOWER_ALLY);
				else if (towers[i].ownership == Tower::Ownership::ENEMY)
					BatchRenderer::DrawTile(towers[i].tile, { tileSize.x,tileSize.y * 2.2f }, TextureID::TOWER_ENEMY);

			}

		}

		glm::vec2 mousePos = Input::GetMousePos();
		if (stage == Stage::UNIT_PLACEMENT)
		{

			TextureID unitTextureID = TextureID::COLOR_RED;
			switch (selectedType)
			{
			case UnitType::INFANTRY: {
				unitTextureID = TextureID::INFANTRY;
				break;
			}
			case UnitType::ARTILLERY: {
				unitTextureID = TextureID::ARTILLERY;
				break;
			}
			case UnitType::SHIELD: {
				unitTextureID = TextureID::SHIELD;
				break;
			}
			}
			if (!isHost)
				BatchRenderer::DrawTile(glm::uvec2{ mapSize - 1, mapSize - 1 } - hoveringTile, { tileSize.x, tileSize.y * 1.5f}, unitTextureID);
			else
				BatchRenderer::DrawTile(hoveringTile, { tileSize.x, tileSize.y * 1.5f }, unitTextureID);

			for (UI& ui : unitSelectionUI)
			{
				glm::vec2 pos = UItoNormalPos(ui.anchorPoint, ui.pos, ui.size);
				if (pos.x + ui.size.x >= mousePos.x && pos.x <= mousePos.x &&
					pos.y + ui.size.y >= mousePos.y && pos.y <= mousePos.y &&
					ui.uiType == UI::Type::BUTTON)
				{

					BatchRenderer::DrawUI(ui.anchorPoint, ui.pos, ui.size * 0.8f, ui.textureID);
					if (ui.text.size())
					{
						BatchRenderer::DrawText(pos + ui.size / 2.0f, 20 * 0.8f, ui.text.c_str());
					}
				}
				else
				{
					BatchRenderer::DrawUI(ui.anchorPoint, ui.pos, ui.size, ui.textureID);
					if (ui.text.size())
					{
						BatchRenderer::DrawText(pos + ui.size / 2.0f, 20, ui.text.c_str());
					}
				}
			}
		}

		BatchRenderer::EndScene();
	}
}