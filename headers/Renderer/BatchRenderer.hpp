#pragma once
#include "RendererCommon.hpp"
#include "UI.hpp"
#include "Texture.hpp"

namespace gal {

	class BatchRenderer
	{
	public:


		static void Init();

		static void BeginScene();
		static void AddStaticQuad(glm::vec2 v2Pos, glm::vec2 v2Size, u32 uTextureID);
		static void ClearStaticQuads();
		static void UploadStaticQuads();
		static void UpdateCamera(glm::vec2 newCam);
		static void DrawText(glm::vec2 v2Pos, float fontSize, const char* pText);
		static void DrawQuad(glm::vec2 v2Pos, glm::vec2 v2Size, u32 uTextureID);
		static void DrawQuad(glm::vec2 v2Pos, glm::vec2 v2Size, TextureID tID);
		static void DrawTile(glm::uvec2 v2Tile, glm::vec2 v2Size, u32 uTextureID);
		static void DrawTile(glm::uvec2 v2Tile, glm::vec2 v2Size, TextureID tID);
		static void DrawUI(UI::Anchor anchorPoint, glm::vec2 v2Pos, glm::vec2 v2Size, u32 uTextureID);
		static void DrawUI(UI::Anchor anchorPoint, glm::vec2 v2Pos, glm::vec2 v2Size, TextureID tId);
		static void EndScene();
		static void Flush();
	};

}