#include "App/MainMenuState.hpp"
#include "Renderer/BatchRenderer.hpp"
#include "App/ArenaState.hpp"
#include "App/Input.hpp"
#include "App/PreArenaState.hpp"
#include "Utility/SoundManager.hpp"

namespace gal {

	enum class SwitchType
	{
		GAMESTATE,
		MENU
	};


	struct ButtonSwitchInfo
	{
		MainMenuState* pState;
		union
		{
			struct
			{
				GameState* pNewGameState;
				GameState::Action action;
			};
			std::vector<UI>* pMenu;
		};
		SwitchType sType;
	};

	void MainMenuState::Start() {

		SoundManager::Play("sounds/background.mp3");

		UI ui = {};
		//Main Menu

		ui.anchorPoint = UI::Anchor::MM;
		ui.size = { 400,100 };
		ui.pos = { 0, 250 };
		ui.textureID = TextureID::EXIT_BUTTON;
		ui.uiType = UI::Type::BUTTON;
		ui.buttonData.hoverTextureID = TextureID::COLOR_RED;
		ui.buttonData.pButtonFunc = [](void* pData) {
			((ButtonSwitchInfo*)pData)->sType = SwitchType::GAMESTATE;
			((ButtonSwitchInfo*)pData)->action = Action::Return;
			((ButtonSwitchInfo*)pData)->pNewGameState = nullptr;
		};
		ui.text = "Exit";

		mainMenu.push_back(ui);
		ui.pos = { 0, 100 };
		ui.textureID = TextureID::CONNECT_BUTTON;
		ui.text = "Connect";
		ui.buttonData.pButtonFunc = [](void* pData) {

			((ButtonSwitchInfo*)pData)->sType = SwitchType::MENU;
			((ButtonSwitchInfo*)pData)->pMenu = &((ButtonSwitchInfo*)pData)->pState->connectionMenu;
		};
		


		mainMenu.push_back(ui);
		ui.text = "Host";
		ui.pos = { 0,-50 };
		ui.textureID = TextureID::HOST_BUTTON;
		ui.buttonData.pButtonFunc = [](void* pData) {

			((ButtonSwitchInfo*)pData)->sType = SwitchType::GAMESTATE;
			((ButtonSwitchInfo*)pData)->action= Action::Switch;
			((ButtonSwitchInfo*)pData)->pNewGameState = new PreArenaState(true);
		};
		mainMenu.push_back(ui);

		// Connection Menu

		ui.pos = {};
		ui.text = "";
		ui.textureID = TextureID::TYPING_PANEL;
		ui.uiType = UI::Type::TEXT_INPUT;
		ui.pBackgroundText = "IP Address...";
		
		connectionMenu.push_back(ui);

		ui.pos = { 0, 150 };
		ui.uiType = UI::Type::BUTTON;
		ui.buttonData.pButtonFunc = [](void* pData) {
			
			((ButtonSwitchInfo*)pData)->sType = SwitchType::GAMESTATE;
			((ButtonSwitchInfo*)pData)->action = Action::Switch;
			((ButtonSwitchInfo*)pData)->pNewGameState = new PreArenaState(false, ((ButtonSwitchInfo*)pData)->pState->pTypingTarget->text.c_str());
		};

		ui.textureID = TextureID::CONNECT_BUTTON_2;
		ui.text = "Connect";
		connectionMenu.push_back(ui);

		//Host Menu


		currentMenu = &mainMenu;
	}

	GameState* MainMenuState::Input(Action* pAction)
	{
		*pAction = Action::None;
		if (Input::GetMouseButton(0) == KeyState::Press) {
			const glm::vec2 mousePos = Input::GetMousePos();
			ButtonSwitchInfo info = {};
			info.pState = this;
			for (UI& ui : *currentMenu) {
				glm::vec2 pos = UItoNormalPos(ui.anchorPoint, ui.pos, ui.size);
				if (pos.x + ui.size.x >= mousePos.x && pos.x <= mousePos.x &&
					pos.y + ui.size.y >= mousePos.y && pos.y <= mousePos.y) {
					if (ui.uiType == UI::Type::BUTTON)
					{
						SoundManager::Play("sounds/Effects/button_click.mp3");

						ui.buttonData.pButtonFunc(&info);

						if (info.sType == SwitchType::GAMESTATE) {
							*pAction = info.action;
							return info.pNewGameState;
						}
						else if (info.sType == SwitchType::MENU) {
							currentMenu = info.pMenu;
							return nullptr;
						}
					}
					else if (ui.uiType == UI::Type::TEXT_INPUT)
					{
						pTypingTarget = &ui;
					}

				}
			}
		}

		if (pTypingTarget) {

			if (Input::GetKey(GLFW_KEY_BACKSPACE) == KeyState::Press)
			{
				if (!pTypingTarget->text.empty())
				{
					//if (pTypingTarget->text.back() == '.')
					//	pTypingTarget->text.pop_back();
					pTypingTarget->text.pop_back();
				}
			}
			else if (pTypingTarget->text.size() < 15) {
				for (int i = GLFW_KEY_SPACE; i <= GLFW_KEY_Z; i++) {
					if (Input::GetKey(i) == KeyState::Press)
					{
						pTypingTarget->text.push_back(i);
						//if (pTypingTarget->text.size() % 3 == pTypingTarget->text.size() / 3 - 1&&
						//	pTypingTarget->text.size() != 0)
						//{
						//	pTypingTarget->text.push_back('.');
						//}
					}
				}
			}
		}
		return nullptr;
	}


	void MainMenuState::Render()
	{
		const glm::vec2 mousePos = Input::GetMousePos();
		BatchRenderer::BeginScene();
		f32 minRes = std::min(Window::GetResolution().x, Window::GetResolution().y);
		f32 maxRes = std::max(Window::GetResolution().x, Window::GetResolution().y);
		BatchRenderer::DrawQuad({ (maxRes - minRes) / 2.0f ,0 }, {minRes,minRes}, TextureID::BACKGROUND);
		for (UI& ui : *currentMenu)
		{
			const glm::vec2 pos = UItoNormalPos(ui.anchorPoint, ui.pos, ui.size);
			if (pos.x + ui.size.x >= mousePos.x && pos.x <= mousePos.x &&
				pos.y + ui.size.y >= mousePos.y && pos.y <= mousePos.y &&
				ui.uiType == UI::Type::BUTTON) {
				BatchRenderer::DrawUI(ui.anchorPoint, ui.pos, ui.size * 0.8f, ui.textureID);
				BatchRenderer::DrawText(pos + ui.size / 2.0f, 25.0f * 0.8f, ui.text.c_str());
			}
			else {
				BatchRenderer::DrawUI(ui.anchorPoint, ui.pos, ui.size, ui.textureID);
				if(ui.text.size() != 0)
					BatchRenderer::DrawText(pos + ui.size / 2.0f, 25.0f, ui.text.c_str());
				else if(ui.uiType == UI::Type::TEXT_INPUT)
					BatchRenderer::DrawText(pos + ui.size / 2.0f, 25.0f, ui.pBackgroundText);
			}
		}
		BatchRenderer::EndScene();
	}
}
