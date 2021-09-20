#include "App/App.hpp"
#include "Renderer/Shader.hpp"
#include "App/Time.hpp"
#include "Renderer/BatchRenderer.hpp"
#include "Utility/TextureManager.hpp"
#include "App/Input.hpp"
#include <iostream>
#include <chrono>
namespace gal
{

	App::App(GameState* gameState)
		:m_GameState(gameState)
	{
		Window::Init();
		BatchRenderer::Init();
		TextureManager::Init();
		glfwSetWindowUserPointer(Window::GetWindowHandle(), this);
		m_GameState->Start();
	}

	App::App(u32 ResolutionX, u32 ResolutionY, GameState* gameState)
		:m_GameState(gameState)
	{
		Window::Init(ResolutionX, ResolutionY);
		BatchRenderer::Init();
		TextureManager::Init();
		glfwSetWindowUserPointer(Window::GetWindowHandle(), this);
		m_GameState->Start();


	}

	void App::Stop()
	{
		Window::m_bIsRunning = false;
	}

	bool App::HandleAction(GameState::Action action, GameState* pGameState)
	{
		switch (action)
		{
			case GameState::Action::None:
			{
				return false;
			}break;
			case GameState::Action::Enter:
			{
				m_sStateStack.push(m_GameState);
				m_GameState = pGameState;
				m_GameState->Start();
			}break;
			case GameState::Action::Return:
			{
				delete m_GameState;
				if (m_sStateStack.empty())
				{
					glfwSetWindowShouldClose(Window::m_pGLwindow, true);
					break;
				}
				m_GameState = m_sStateStack.top();
				m_GameState->Focus();
				m_sStateStack.pop();
			}break;
			case GameState::Action::Switch:
			{
				delete m_GameState;
				m_GameState = pGameState;
				m_GameState->Start();
			}break;
		}
		return true;
	}

	void App::Run()
	{
		//glfwSwapInterval(1);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		float time = 0;
		do
		{
			glClear(GL_COLOR_BUFFER_BIT);
			Time::Step();
			Input::ScanKeys();
			Window::PollEvents();

			//std::cout <<"FPS: "<< 1.0f / Time::DeltaTime()<<"\r";

			GameState::Action act;
			GameState* pNewState;

			pNewState = m_GameState->Input(&act);
			if (HandleAction(act, pNewState))
				continue;
			pNewState = m_GameState->Update(&act);
			if (HandleAction(act, pNewState))
				continue; 
			pNewState = m_GameState->LateUpdate(&act);
			if (HandleAction(act, pNewState))
				continue;
			m_GameState->Render();
			Window::SwapBuffers();

		} while (Window::IsRunning());
	}
}