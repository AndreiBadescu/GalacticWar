#pragma once

namespace gal {

	class GameState {
	public:
		enum class Action
		{
			None, Switch, Return, Enter
		};
	public:

		virtual void Start() = 0;
		virtual GameState* Input(GameState::Action* pAction) = 0;
		virtual GameState* Update(GameState::Action* pAction) = 0;
		virtual GameState* LateUpdate(GameState::Action* pAction) = 0;
		virtual void Render() = 0;
		virtual void Focus() {}

	};

}