#pragma once
#include <vector>
#include <algorithm>
#include "RendererCommon.hpp"
#include "Renderer/Texture.hpp"
#include "deps/stb_truetype/stb_truetype.h"

namespace gal
{

	class TextureManager
	{
	public:
		static constexpr float fontHeight = 100.0f;
		static constexpr float fontWidth = 200.0f;
		static constexpr unsigned int fontPixelsY = fontHeight * 10;
		static constexpr unsigned int fontPixelsX = fontWidth * 10;

		static void Init();
		static void Shutdown();
		static void AddTexture(const char* pFilePath);
		static void AddTexture(TextureID textureID, const char* pFilePath);
		static void AddFont(TextureID textureID, const char* pFilePath);
		static void AddFont(const char* pFilePath);
		static void AddColor(glm::vec4 vColor);
		static void AddColor(TextureID textureID, glm::vec4 vColor);
		static u32 BindStaticTexture(u32 ID);
		static u32 BindTexture(u32 ID);
		static void ClearStaticTextures();
		static void Flush();
		static const stbtt_bakedchar& GetCharInfo(char ch);
	private:
		static std::vector<Texture> vTextures;
		static u64 uStaticTextureCount;
		static stbtt_bakedchar characters[96];
	};
}