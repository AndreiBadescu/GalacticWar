#include "App/Input.hpp"
#include "App/Window.hpp"
#include "RendererCommon.hpp"
#include <GLFW/glfw3.h>

namespace gal
{
	int Input::m_iKeys[99];
	int Input::m_iMouseButtons[2];
	glm::vec2 Input::m_MousePosition;


	internal_func KeyState glfwKeyStateToGalKeyState(GLint keyState)
	{
		switch (keyState)
		{
		case GLFW_PRESS:
			return KeyState::Press;
		case GLFW_RELEASE:
			return KeyState::Release;
		case GLFW_REPEAT:
			return KeyState::Hold;
		default:
			return KeyState::None;
		}
	}


	void Input::Init()
	{
		memset(m_iKeys, -1, sizeof(m_iKeys));
		m_iMouseButtons[0] = -1;
		m_iMouseButtons[1] = -1;
	}
	void Input::SetMouseButton(int button, int state)
	{
		m_iMouseButtons[button] = state;
	}

	KeyState Input::GetMouseButton(int button)
	{
		return glfwKeyStateToGalKeyState(m_iMouseButtons[button]);
	}

	void Input::SetKey(int key, int state)
	{
		if (key >= GLFW_KEY_SPACE && key <= GLFW_KEY_Z)
			m_iKeys[key - GLFW_KEY_SPACE] = state;
		else if (key == GLFW_KEY_BACKSPACE)
			m_iKeys[96] = state;
		else if (key == GLFW_KEY_ENTER)
			m_iKeys[97] = state;
		else if (key == GLFW_KEY_ESCAPE)
			m_iKeys[98] = state;
	}

	KeyState Input::GetKey(int key)
	{
		if (key == GLFW_KEY_BACKSPACE)
			return glfwKeyStateToGalKeyState(m_iKeys[96]);
		else if (key == GLFW_KEY_ENTER)			 
			return glfwKeyStateToGalKeyState(m_iKeys[97]);
		else if (key == GLFW_KEY_ESCAPE)		 
			return glfwKeyStateToGalKeyState(m_iKeys[98]);
		else
			return glfwKeyStateToGalKeyState(m_iKeys[key - GLFW_KEY_SPACE]);
	}

	void Input::ScanKeys()
	{
		if (m_iMouseButtons[0] == GLFW_PRESS)
		{
			m_iMouseButtons[0] = GLFW_REPEAT;
		}
		else if (m_iMouseButtons[0] == GLFW_RELEASE)
		{
			m_iMouseButtons[0] = -1;
		}

		if (m_iMouseButtons[1] == GLFW_PRESS)
		{
			m_iMouseButtons[1] = GLFW_REPEAT;
		}
		else if (m_iMouseButtons[1] == GLFW_RELEASE)
		{
			m_iMouseButtons[1] = -1;
		}

		for (int i = 0; i < 99; i++)
		{
			if (m_iKeys[i] == GLFW_RELEASE)
			{
				m_iKeys[i] = -1;
			}
			else if (m_iKeys[i] == GLFW_PRESS)
			{
				m_iKeys[i] = GLFW_REPEAT;
			}
		}
	}

	bool Input::MouseIsHovering(glm::vec2 pos, glm::vec2 size)
	{
		bool b = m_MousePosition.x >= pos.x - size.x / 2.0f && m_MousePosition.x <= pos.x + size.x / 2.0f &&
			m_MousePosition.y >= pos.y - size.y / 2.0f && m_MousePosition.y <= pos.y + size.y / 2.0f;
		return b;
	}

}