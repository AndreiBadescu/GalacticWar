#include "App/ArenaState.hpp"
#include "Renderer/BatchRenderer.hpp"
#include "Utility/Math.hpp"
#include "App/GameLogic.hpp"
#include "App/Input.hpp"
#include "App/Time.hpp"
#include "Renderer/UI.hpp"
#include "P2P/NetworkInterface.hpp"
#include "Utility/SoundManager.hpp"

namespace gal
{
	ArenaState::ArenaState(PreArenaState* pPreArenaState)
	{
		yourUnits = std::move(pPreArenaState->yourUnits);
		enemyUnits = std::move(pPreArenaState->enemyUnits);
		socketThread = std::move(pPreArenaState->socketThread);

		memcpy(tile_map, pPreArenaState->tile_map, sizeof(tile_map));
		memcpy(obst_map, pPreArenaState->obst_map, sizeof(obst_map));
		memcpy(towers, pPreArenaState->towers, sizeof(towers));

		isHost = pPreArenaState->isHost;
		pMainCanModify = pPreArenaState->pMainCanModify;
		pPacket = pPreArenaState->pPacket;
		pSocketCv = pPreArenaState->pSocketCv;
		pSocketMutex = pPreArenaState->pSocketMutex;
		yourTurn = pPreArenaState->yourTurn;
	}

	void ArenaState::Start()
	{
		money = 0;
		unitIsEnemy = false;
		m_camera = { 0,0 };
		selectedUnitIndex = 0;
		uSelectedTowerIndex = 0;
		moneyUI = {};
		moneyUI.anchorPoint = UI::Anchor::TR;
		moneyUI.size = {100,100};
		moneyUI.textureID = TextureID::DOGE_COIN;
		moneyUI.uiType = UI::Type::IMAGE;
		UI ui{};

		ui.anchorPoint = UI::Anchor::TM;
		ui.size = { 200,100 };
		ui.pos = { -200,0 };
		ui.textureID = TextureID::CONNECT_BUTTON_2;
		ui.uiType = UI::Type::BUTTON;
		ui.text = "Infantry";
		ui.buttonData.pButtonFunc = [](void* pData) {

			*(UnitType*)pData = UnitType::INFANTRY;
		};

		towerShopMenu.emplace_back(ui);
		ui.pos = {};
		ui.text = "Artillery";
		ui.buttonData.pButtonFunc = [](void* pData) {

			*(UnitType*)pData = UnitType::ARTILLERY;
		};

		towerShopMenu.emplace_back(ui);
		ui.pos = { 200,0 };
		ui.text = "Shield";
		ui.buttonData.pButtonFunc = [](void* pData) {

			*(UnitType*)pData = UnitType::SHIELD;
		};

		towerShopMenu.emplace_back(ui);


		endTurnButton = {};
		endTurnButton.anchorPoint = UI::Anchor::BR;
		endTurnButton.uiType = UI::Type::BUTTON;
		endTurnButton.size = { 300,200 };
		endTurnButton.buttonData.pButtonFunc = [](void* pData) {
			*(bool*)pData = true;
		};
		endTurnButton.textureID = TextureID::END_TURN_BUTTON;
	}

	GameState* ArenaState::Input(GameState::Action* pAction)
	{
		*pAction = GameState::Action::None;
		const glm::vec2 mousePos = Input::GetMousePos();
		glm::uvec2 hoveringTile = GetHoveringTile();

		if (!isHost)
		{
			hoveringTile.x = mapSize - 1 - hoveringTile.x;
			hoveringTile.y = mapSize - 1 - hoveringTile.y;
		}

		if (Input::GetKey(GLFW_KEY_A) == KeyState::Press) {
			m_camera.x++;
		}
		else if (Input::GetKey(GLFW_KEY_D) == KeyState::Press) {
			m_camera.x--;
		}
		if (Input::GetKey(GLFW_KEY_W) == KeyState::Press) {
			m_camera.y++;
		}
		else if (Input::GetKey(GLFW_KEY_S) == KeyState::Press) {
			m_camera.y--;
		}



		if (yourTurn)
		{
			if (*pMainCanModify)
			{
				if (Input::GetMouseButton(0) == KeyState::Press)
				{
					HandleLeftClick(*this);
				}

				if (Input::GetMouseButton(1) == KeyState::Press)
				{
					HandleRightClick(*this);
				}
			}
		}
		UpdateMapConstants(m_camera);

		return nullptr;
	}

	GameState* ArenaState::Update(GameState::Action* pAction)
	{
		*pAction = GameState::Action::None;
		const glm::vec2 mousePos = Input::GetMousePos();
		glm::uvec2 hoveringTile = GetHoveringTile();

		if (!isHost)
		{
			hoveringTile.x = mapSize - 1 - hoveringTile.x;
			hoveringTile.y = mapSize - 1 - hoveringTile.y;
		}

		if (!yourTurn) {
			if (*pMainCanModify) {
				Packet temp = *pPacket;
				*pMainCanModify = false;
				pSocketCv->notify_one();

				switch (temp.function_id) {
				case FunctionId::ATTACK: {
					DecodeAttack(*this, temp);
				}break;
				case FunctionId::MOVE: {
					DecodeMove(*this, temp);
				}break;
				case FunctionId::GIVE_TURN: {
					yourTurn = true;
				}break;

				case FunctionId::CAPTURE: {
					DecodeCapture(*this, temp);
				}break;

				case FunctionId::BUY: {
					DecodeBuy(*this, temp);
				}break;
				}
			}
		}

		return nullptr;
	}



	void ArenaState::Render()
	{
		glm::uvec2 hoveringTile = GetHoveringTile();
		const glm::vec2 mousePos = Input::GetMousePos();
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
					else if (obst_map[y][x] == TileObst::TREE)
						BatchRenderer::DrawTile(glm::uvec2{ mapSize - 1, mapSize - 1 } - glm::uvec2{ x,y }, { tileSize.x, tileSize.y * 2.0f }, (u32)Tile::MAX_TILE + (u32)obst_map[y][x]);
					else
						BatchRenderer::DrawTile(glm::uvec2{ mapSize - 1, mapSize - 1 } - glm::uvec2{ x,y }, tileSize, (u32)Tile::MAX_TILE + (u32)obst_map[y][x]);
				}
			}


			for (int i = 0; i < MAX_TOWERS; i++)
			{
				if (towers[i].ownership == Tower::Ownership::NONE)
					BatchRenderer::DrawTile(glm::uvec2{ mapSize - 1, mapSize - 1 } - towers[i].tile, { tileSize.x,tileSize.y * 2.2f }, TextureID::TOWER_OFF);
				else if (towers[i].ownership == Tower::Ownership::ALLY)
					BatchRenderer::DrawTile(glm::uvec2{ mapSize - 1, mapSize - 1 } - towers[i].tile, { tileSize.x,tileSize.y * 2.2f }, TextureID::TOWER_ALLY);
				else if (towers[i].ownership == Tower::Ownership::ENEMY)
					BatchRenderer::DrawTile(glm::uvec2{ mapSize - 1, mapSize - 1 } - towers[i].tile, { tileSize.x,tileSize.y * 2.2f }, TextureID::TOWER_ENEMY);

			}

			for (Unit& un : enemyUnits)
			{
				float fPercent = (float)un.healthPoints;
				switch (un.unitType)
				{
				case UnitType::INFANTRY: {
					fPercent /= (float)UNIT_INFANTRY.healthPoints;
				}break;
				case UnitType::ARTILLERY: {
					fPercent /= (float)UNIT_ARTILLERY.healthPoints;
				}break;
				case UnitType::SHIELD: {
					fPercent /= (float)UNIT_SHIELD.healthPoints;
				}break;
				}

				glm::vec2 pos = TileToPixels(glm::uvec2{ mapSize - 1 - un.tile.x, un.tile.y });
				pos.x -= ((1.0f - fPercent) / 2.0f) * tileSize.x * 1.2f;
				pos.y += tileSize.y * 2.0f;
				BatchRenderer::DrawQuad(pos, { tileSize.x * 1.2f * fPercent, tileSize.y * 0.2f }, TextureID::COLOR_RED);
			}

			for (Unit& un : yourUnits)
			{
				float fPercent = (float)un.healthPoints;
				switch (un.unitType)
				{
				case UnitType::INFANTRY: {
					fPercent /= (float)UNIT_INFANTRY.healthPoints;
				}break;
				case UnitType::ARTILLERY: {
					fPercent /= (float)UNIT_ARTILLERY.healthPoints;
				}break;
				case UnitType::SHIELD: {
					fPercent /= (float)UNIT_SHIELD.healthPoints;
				}break;
				}

				glm::vec2 pos = TileToPixels(glm::uvec2{ mapSize - 1 - un.tile.x, un.tile.y });
				pos.x -= ((1.0f - fPercent) / 2.0f) * tileSize.x * 1.2f;
				pos.y += tileSize.y * 2.0f;
				BatchRenderer::DrawQuad(pos, { tileSize.x * 1.2f * fPercent, tileSize.y * 0.2f }, TextureID::COLOR_GREEN);
			}


			for (glm::uvec2 tile : moveRangeTiles) {
				bool isHosTile = false;
				for (Unit& eUn : enemyUnits)
				{
					if (eUn.tile == tile)
					{
						isHosTile = true;
						break;
					}
				}
				if (!isHosTile)
					BatchRenderer::DrawTile(glm::uvec2{ mapSize - 1, mapSize - 1 } - tile, tileSize, TextureID::COLOR_TRANSPARENT_GREEN);
			}

			for (glm::uvec2 tile : attackRangeTiles) {
				bool isHosTile = false;
				for (Unit& eUn : enemyUnits)
				{
					if (eUn.tile == tile)
					{
						isHosTile = true;
						break;
					}
				}
				if (isHosTile)
					BatchRenderer::DrawTile(glm::uvec2{ mapSize - 1, mapSize - 1 } - tile, tileSize, TextureID::COLOR_TRANSPARENT_RED);
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
					else if (obst_map[y][x] == TileObst::TREE)
						BatchRenderer::DrawTile(glm::uvec2{ x,y }, { tileSize.x, tileSize.y * 2.0f }, (u32)Tile::MAX_TILE + (u32)obst_map[y][x]);
					else
						BatchRenderer::DrawTile(glm::uvec2{ x,y }, tileSize, (u32)Tile::MAX_TILE + (u32)obst_map[y][x]);
				}
			}


			for (int i = 0; i < MAX_TOWERS; i++)
			{
				if (towers[i].ownership == Tower::Ownership::NONE)
					BatchRenderer::DrawTile(towers[i].tile, { tileSize.x,tileSize.y * 2.2f }, TextureID::TOWER_OFF);
				else if (towers[i].ownership == Tower::Ownership::ALLY)
					BatchRenderer::DrawTile(towers[i].tile, { tileSize.x,tileSize.y * 2.2f }, TextureID::TOWER_ALLY);
				else if (towers[i].ownership == Tower::Ownership::ENEMY)
					BatchRenderer::DrawTile(towers[i].tile, { tileSize.x,tileSize.y * 2.2f }, TextureID::TOWER_ENEMY);

			}

			for (Unit& un : enemyUnits)
			{
				float fPercent = (float)un.healthPoints;
				switch (un.unitType)
				{
				case UnitType::INFANTRY: {
					fPercent /= (float)UNIT_INFANTRY.healthPoints;
				}break;
				case UnitType::ARTILLERY: {
					fPercent /= (float)UNIT_ARTILLERY.healthPoints;
				}break;
				case UnitType::SHIELD: {
					fPercent /= (float)UNIT_SHIELD.healthPoints;
				}break;
				}
				glm::vec2 pos = TileToPixels(glm::uvec2{ un.tile.x, mapSize - 1 - un.tile.y });
				pos.x -= ((1.0f - fPercent) / 2.0f) * tileSize.x * 1.2f;
				pos.y += tileSize.y * 2.0f;
				BatchRenderer::DrawQuad(pos, { tileSize.x * 1.2f * fPercent, tileSize.y * 0.2f }, TextureID::COLOR_RED);
			}

			for (Unit& un : yourUnits)
			{
				float fPercent = (float)un.healthPoints;
				switch (un.unitType)
				{
				case UnitType::INFANTRY: {
					fPercent /= (float)UNIT_INFANTRY.healthPoints;
				}break;
				case UnitType::ARTILLERY: {
					fPercent /= (float)UNIT_ARTILLERY.healthPoints;
				}break;
				case UnitType::SHIELD: {
					fPercent /= (float)UNIT_SHIELD.healthPoints;
				}break;
				}

				glm::vec2 pos = TileToPixels(glm::uvec2{ un.tile.x, mapSize - 1 - un.tile.y });
				pos.x -= ((1.0f - fPercent) / 2.0f) * tileSize.x * 1.2f;
				pos.y += tileSize.y * 2.0f;
				BatchRenderer::DrawQuad(pos, { tileSize.x * 1.2f * fPercent, tileSize.y * 0.2f }, TextureID::COLOR_GREEN);
			}



			for (glm::uvec2 tile : moveRangeTiles) {
				bool isHosTile = false;
				for (Unit& eUn : enemyUnits)
				{
					if (eUn.tile == tile)
					{
						isHosTile = true;
						break;
					}
				}
				if (!isHosTile)
					BatchRenderer::DrawTile(tile, tileSize, TextureID::COLOR_TRANSPARENT_GREEN);
			}

			for (glm::uvec2 tile : attackRangeTiles) {
				bool isHosTile = false;
				for (Unit& eUn : enemyUnits)
				{
					if (eUn.tile == tile)
					{
						isHosTile = true;
						break;
					}
				}
				if (isHosTile)
					BatchRenderer::DrawTile(tile, tileSize, TextureID::COLOR_TRANSPARENT_RED);
			}

		}

		if (selectedUnitIndex != 0)
		{
			glm::vec2 pos = UItoNormalPos(UI::Anchor::BL, { 0,0 }, { 400,400 });
			BatchRenderer::DrawQuad(pos, { 400,400 }, TextureID::UNIT_PANEL);
			if (unitIsEnemy == false)
			{
				Unit& un = yourUnits[selectedUnitIndex - 1];
				char buffer[255];
				sprintf(buffer, "AP:%u", un.actionPoints);
				BatchRenderer::DrawText(pos + glm::vec2(200, 60), 20, buffer);
				sprintf(buffer, "HP:%u", un.healthPoints);
				BatchRenderer::DrawText(pos + glm::vec2(200, 120), 20, buffer);
				sprintf(buffer, "DMG:%u", un.damagePoints);
				BatchRenderer::DrawText(pos + glm::vec2(200, 180), 20, buffer);
				switch (un.unitType)
				{
				case UnitType::INFANTRY: {
					BatchRenderer::DrawText(pos + glm::vec2(200, 240), 20, "Type:Infantry");
				} break;
				case UnitType::ARTILLERY: {
					BatchRenderer::DrawText(pos + glm::vec2(200, 240), 20, "Type:Artillery");
				} break;
				case UnitType::SHIELD: {
					BatchRenderer::DrawText(pos + glm::vec2(200, 240), 20, "Type:Shield");
				} break;
				}
			}
			else
			{
				Unit& un = enemyUnits[selectedUnitIndex - 1];
				char buffer[255];
				sprintf(buffer, "AP:%u", un.actionPoints);
				BatchRenderer::DrawText(pos + glm::vec2(200, 60), 20, buffer);
				sprintf(buffer, "HP:%u", un.healthPoints);
				BatchRenderer::DrawText(pos + glm::vec2(200, 120), 20, buffer);
				sprintf(buffer, "DMG:%u", un.damagePoints);
				BatchRenderer::DrawText(pos + glm::vec2(200, 180), 20, buffer);
				switch (un.unitType)
				{
				case UnitType::INFANTRY: {
					BatchRenderer::DrawText(pos + glm::vec2(200, 240), 20, "Type:Infantry");
				} break;
				case UnitType::ARTILLERY: {
					BatchRenderer::DrawText(pos + glm::vec2(200, 240), 20, "Type:Artillery");
				} break;
				case UnitType::SHIELD: {
					BatchRenderer::DrawText(pos + glm::vec2(200, 240), 20, "Type:Shield");
				} break;
				}
			}
		}
		else if (uSelectedTowerIndex != 0)
		{

			for (UI& ui : towerShopMenu)
			{
				const glm::vec2 pos = UItoNormalPos(ui.anchorPoint, ui.pos, ui.size);
				if (pos.x + ui.size.x >= mousePos.x && pos.x <= mousePos.x &&
					pos.y + ui.size.y >= mousePos.y && pos.y <= mousePos.y &&
					ui.uiType == UI::Type::BUTTON)
				{
					BatchRenderer::DrawUI(ui.anchorPoint, ui.pos, ui.size * 0.8f, ui.textureID);
					if (ui.text.size())
					{
						BatchRenderer::DrawText(pos + ui.size / 2.0f, 25.0f * 0.8f, ui.text.c_str());
					}
				}
				else
				{
					BatchRenderer::DrawUI(ui.anchorPoint, ui.pos, ui.size, ui.textureID);
					if (ui.text.size())
					{
						BatchRenderer::DrawText(pos + ui.size / 2.0f, 25.0f, ui.text.c_str());
					}
				}
			}
		}

		if (yourTurn)
		{
			const glm::vec2 pos = UItoNormalPos(endTurnButton.anchorPoint, endTurnButton.pos, endTurnButton.size);
			if (pos.x + endTurnButton.size.x >= mousePos.x && pos.x <= mousePos.x &&
				pos.y + endTurnButton.size.y >= mousePos.y && pos.y <= mousePos.y &&
				endTurnButton.uiType == UI::Type::BUTTON)
			{
				BatchRenderer::DrawUI(endTurnButton.anchorPoint, endTurnButton.pos, endTurnButton.size * 0.8f, endTurnButton.textureID);
				if (endTurnButton.text.size())
				{
					BatchRenderer::DrawText(pos + endTurnButton.size / 2.0f, 25.0f * 0.8f, endTurnButton.text.c_str());
				}
			}
			else
			{
				BatchRenderer::DrawUI(endTurnButton.anchorPoint, endTurnButton.pos, endTurnButton.size, endTurnButton.textureID);
				if (endTurnButton.text.size())
				{
					BatchRenderer::DrawText(pos + endTurnButton.size / 2.0f, 25.0f, endTurnButton.text.c_str());
				}
			}
		}

		{
			char buffer[255];
			glm::vec2 pos = UItoNormalPos(moneyUI.anchorPoint, moneyUI.pos, moneyUI.size);
			BatchRenderer::DrawUI(moneyUI.anchorPoint, moneyUI.pos, moneyUI.size, moneyUI.textureID);
			sprintf(buffer, "%u", money);
			BatchRenderer::DrawText(pos , 20, buffer);
		}
		BatchRenderer::EndScene();
	}
}