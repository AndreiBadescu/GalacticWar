#include "App/Window.hpp"
#include "App/Input.hpp"
#include "Utility/Math.hpp"
#include <stdexcept>
#include <iostream>

namespace gal {
	static constexpr char pWindowTitle[] = "Galactic War";

	bool Window::m_bIsRunning = true;
	GLFWwindow* Window::m_pGLwindow = nullptr;
	glm::uvec2 Window::m_uResolution;

	

	internal_func void windowSizeCallback(GLFWwindow* pWindow, int x, int y)
	{
		Window::SetResolution({ x,y });
		glViewport(0, 0, x, y);
	}



	internal_func void cursorPosCallback(GLFWwindow* pWindow, double x, double y)
	{
		glm::vec2 newPos(x, Window::GetResolution().y - y);
		Input::SetMousePos(newPos);
	}

	internal_func void keyPressCallback(GLFWwindow* pWindow, int key, int scancode, int state, int mods)
	{
		if(state != GLFW_REPEAT)
			Input::SetKey(key, state);
	}

	internal_func void mouseButtonCallback(GLFWwindow* pWindow, int mouseButton, int state, int mods)
	{
		if (state != GLFW_REPEAT && mouseButton < 2)
		{
			Input::SetMouseButton(mouseButton, state);
		}
	}

	glm::vec2 Window::GetAspectRatio()
	{
		glm::vec2 ar{10.0f};
		const float fYRatio = (float)m_uResolution.y / (float)m_uResolution.x;
		ar.y = ar.x * fYRatio;
		return ar;
	}

	void Window::Init()
	{
		

		m_bIsRunning = true;

		if (!glfwInit())
		{
			throw std::runtime_error("Could not initialize GLFW!\n");
		}
		GLFWmonitor* pMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* pVidMode = glfwGetVideoMode(pMonitor);
		m_uResolution.x = pVidMode->width;
		m_uResolution.y = pVidMode->height;
		m_pGLwindow = glfwCreateWindow(m_uResolution.x, m_uResolution.y, pWindowTitle, pMonitor, nullptr);
		glfwSetWindowAspectRatio(m_pGLwindow, 800, 600);
		glfwMakeContextCurrent(m_pGLwindow);
		UpdateMapConstants();
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			throw std::runtime_error("Could not initialize GLAD!\n");
		}
		glfwSetWindowSizeCallback(m_pGLwindow, windowSizeCallback);
		glfwSetCursorPosCallback(m_pGLwindow, cursorPosCallback);
		glfwSetMouseButtonCallback(m_pGLwindow, mouseButtonCallback);
		glfwSetKeyCallback(m_pGLwindow, keyPressCallback);
	}

	void Window::Init(u32 uResX, u32 uResY)
	{

		m_uResolution.x = uResX;
		m_uResolution.y = uResY;

		UpdateMapConstants();

		m_bIsRunning = true;

		if (!glfwInit())
		{
			throw std::runtime_error("Could not initialize GLFW!\n");
		}
		m_pGLwindow = glfwCreateWindow(m_uResolution.x, m_uResolution.y, pWindowTitle, nullptr, nullptr);
		glfwSetWindowAspectRatio(m_pGLwindow, uResX, uResY);
		glfwMakeContextCurrent(m_pGLwindow);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			throw std::runtime_error("Could not initialize GLAD!\n");
		}

		glfwSetWindowSizeCallback(m_pGLwindow, windowSizeCallback);
		glfwSetMouseButtonCallback(m_pGLwindow, mouseButtonCallback);
		glfwSetCursorPosCallback(m_pGLwindow, cursorPosCallback);
		glfwSetKeyCallback(m_pGLwindow, keyPressCallback);
	}

	void Window::Destroy() {
		glfwDestroyWindow(m_pGLwindow);
	}

	bool Window::IsRunning() {
		m_bIsRunning = !glfwWindowShouldClose(m_pGLwindow);
		return m_bIsRunning;
	}

	void Window::PollEvents() {
		glfwPollEvents();
	}

	void Window::SwapBuffers() {
		glfwSwapBuffers(m_pGLwindow);
	}
}