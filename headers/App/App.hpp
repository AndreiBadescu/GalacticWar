#pragma once
#include <thread>
#include <condition_variable>
#include <stack>
#include "Window.hpp"
#include "GameState.hpp"

namespace gal {

	class App {
	public:

		App(GameState* gameState = nullptr);
		App(u32 ResolutionX, u32 ResolutionY, GameState* gameState = nullptr);

		void Run();
		void Stop();
		bool HandleAction(GameState::Action action, GameState* pGameState);


	public:
		GameState* m_GameState;
		std::stack<GameState*> m_sStateStack;
	};
}