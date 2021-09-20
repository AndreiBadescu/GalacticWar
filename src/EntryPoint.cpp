#include "App/App.hpp"
#include "App/Time.hpp"
#include "Utility/Allocation.hpp"
#include "Utility/TextureManager.hpp"
#include "Renderer/BatchRenderer.hpp"
#include <glad/glad.h>
#include <stdexcept>
#include <iostream>
#include <thread>
//#include <Windows.h>
#include "App/ArenaState.hpp"
#include "App/MainMenuState.hpp"
#include "P2P/Host.hpp"
#include "P2P/Client.hpp"
#include "Utility/ErrorHandler.hpp"

//INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
//	PSTR lpCmdLine, INT nCmdShow)
int main()
{
	try
	{
		gal::ArenaAllocator::Init();
		gal::Time::Init();
#ifndef NDEBUG
		gal::App game(1920, 1080, new gal::MainMenuState());
#else
		gal::App game(1920, 1080, new gal::MainMenuState());
		//gal::App game(new gal::ArenaState());
#endif
		game.Run();
	}
	catch (std::runtime_error &e)
	{
		std::cerr << e.what() << '\n';
	}
	
	return 0;
}