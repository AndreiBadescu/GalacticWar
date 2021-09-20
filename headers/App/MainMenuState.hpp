#pragma once
#include "GameState.hpp"
#include "Renderer/UI.hpp"
#include <vector>

namespace gal {

	class MainMenuState : public GameState {
	public: 

		void Start() override;
		GameState* Input(Action* pAction) override;
		GameState* Update(Action* pAction) override { *pAction = Action::None; return nullptr; }
		GameState* LateUpdate(Action* pAction) override { *pAction = Action::None; return nullptr; }
		void Render() override;

	public:
		std::vector<UI>* currentMenu;
		std::vector<UI> mainMenu;
		std::vector<UI> connectionMenu;
		std::vector<UI> hostMenu;
		UI* pTypingTarget;
	};

}
