#pragma once
#include "RendererCommon.hpp"

namespace gal {
	class Window {
	public:

		static void Init();
		static void Init(u32 uResX, u32 uResY);
		static void Destroy();

		static bool IsRunning();
		static void PollEvents();
		static void SwapBuffers();
		
		static GLFWwindow* GetWindowHandle() { return m_pGLwindow; }
		static void SetResolution(glm::uvec2 res) { m_uResolution = res; }
		static glm::uvec2  GetResolution()   { return m_uResolution; }
		static glm::vec2 GetAspectRatio();
	private:
		friend class App;

		static GLFWwindow* m_pGLwindow;
		static glm::uvec2 m_uResolution;
		static bool m_bIsRunning;
	};
}