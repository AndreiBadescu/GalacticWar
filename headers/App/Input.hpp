#pragma once
#include <glm/glm.hpp>


namespace gal
{

	enum class KeyState
	{
		Press, Release, Hold, None
	};



	class Input
	{
	public:

		static void Init();

		static void SetMouseButton(int button, int state);
		static KeyState GetMouseButton(int button);
		static void SetMousePos(glm::vec2 newPos) { m_MousePosition = newPos; }
		static glm::vec2 GetMousePos() { return m_MousePosition; }

		static void SetKey(int key, int state);
		static KeyState GetKey(int key);
		static void ScanKeys();
		static bool MouseIsHovering(glm::vec2 rectPos, glm::vec2 rectSize);
	private:
		static glm::vec2 m_MousePosition;
		static int m_iMouseButtons[2];
		static int m_iKeys[99];
	};
}