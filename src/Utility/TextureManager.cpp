#include "deps/stb_image/stb_image.h"
#include "deps/stb_truetype/stb_truetype.h"
#include "Utility/TextureManager.hpp"
#include <algorithm>
#include <fstream>

namespace gal
{

	static constexpr float fontHeight = 100.0f;
	static constexpr float fontWidth = 200.0f;
	static constexpr unsigned int fontPixelsY = fontHeight * 10;
	static constexpr unsigned int fontPixelsX = fontWidth * 10;

	std::vector<Texture> TextureManager::vTextures;
	u64 TextureManager::uStaticTextureCount = 0;
	stbtt_bakedchar TextureManager::characters[96];

	global_variable u32 uMaxTexture = 0;
	global_variable GLint iTextureUnitCount;
	global_variable u32 uBoundTextures[64];

	void TextureManager::AddTexture(TextureID textureID, const char* pFilePath)
	{
		u32 idIndex = (u32)textureID;
		vTextures[idIndex] = std::move(Texture(pFilePath));
	}

	u32 TextureManager::BindStaticTexture(u32 ID)
	{
		for (int i = 0; i < uMaxTexture; i++)
		{
			if (uBoundTextures[i] == ID)
			{
				return i;
			}
		}

		if (uMaxTexture >= iTextureUnitCount)
			return -1;

		glBindTextureUnit(uMaxTexture, TextureManager::vTextures[ID].m_hTexture);
		uBoundTextures[uMaxTexture] = ID;
		uMaxTexture++;
		uStaticTextureCount++;

		return uMaxTexture - 1;
	}

	void TextureManager::ClearStaticTextures()
	{
		uStaticTextureCount = 0;
	}

	void TextureManager::AddColor(TextureID textureID, glm::vec4 vColor)
	{
		u8 color[4];
		color[0] = vColor.r * 255.0f;
		color[1] = vColor.g * 255.0f;
		color[2] = vColor.b * 255.0f;
		color[3] = vColor.a * 255.0f;

		Texture t;

		glCreateTextures(GL_TEXTURE_2D, 1, &t.m_hTexture);
		glBindTexture(GL_TEXTURE_2D, t.m_hTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, color);
		u32 idIndex = (u32)textureID;
		vTextures[idIndex] = std::move(t);
	}

	void TextureManager::AddColor(glm::vec4 vColor)
	{
		u8 color[4];
		color[0] = vColor.r * 255.0f;
		color[1] = vColor.g * 255.0f;
		color[2] = vColor.b * 255.0f;
		color[3] = vColor.a * 255.0f;

		Texture t;

		glCreateTextures(GL_TEXTURE_2D, 1, &t.m_hTexture);
		glBindTexture(GL_TEXTURE_2D, t.m_hTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, color);
		vTextures.push_back(std::move(t));
	}

	void TextureManager::AddFont(TextureID textureID, const char* pFilePath)
	{
		std::ifstream f(pFilePath, std::ios::ate | std::ios::binary);

		size_t fileSize = f.tellg();
		f.seekg(0, std::ios::beg);

		unsigned char* pBuffer = new unsigned char[fileSize];
		unsigned char* pPixels = new unsigned char[fontPixelsX * fontPixelsY];
		f.read((char*)pBuffer, fileSize);
		int info = stbtt_BakeFontBitmap(pBuffer, 0, 100.0f, pPixels, fontPixelsX, fontPixelsY, ' ', 96, characters);

		delete[] pBuffer;


		unsigned char* pImage = new unsigned char[fontPixelsX * fontPixelsY * 4];
		int pixel = 0;
		for (int i = 0; i < fontPixelsX * fontPixelsY * 4; i += 4, pixel++)
		{
			pImage[i] = 255;
			pImage[i + 1] = 255;
			pImage[i + 2] = 255;
			pImage[i + 3] = 255 * (pPixels[pixel] != 0);
		}
		stbtt_FreeBitmap(pPixels, nullptr);

		pPixels = pImage;
		pImage = nullptr;

		Texture t;

		glCreateTextures(GL_TEXTURE_2D, 1, &t.m_hTexture);
		glBindTexture(GL_TEXTURE_2D, t.m_hTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fontPixelsX, fontPixelsY, 0, GL_RGBA, GL_UNSIGNED_BYTE, pPixels);

		delete[] pPixels;
		u32 idIndex = (u32)textureID;
		vTextures[idIndex] = std::move(t);
	}

	void TextureManager::AddFont(const char* pFilePath)
	{
		std::ifstream f(pFilePath, std::ios::ate | std::ios::binary);

		size_t fileSize = f.tellg();
		f.seekg(0, std::ios::beg);

		unsigned char* pBuffer = new unsigned char[fileSize];
		unsigned char* pPixels = new unsigned char[fontPixelsX * fontPixelsY];
		f.read((char*)pBuffer, fileSize);
		int info = stbtt_BakeFontBitmap(pBuffer, 0, 100.0f, pPixels, fontPixelsX, fontPixelsY, ' ', 96, characters);

		delete[] pBuffer;


		unsigned char* pImage = new unsigned char[fontPixelsX * fontPixelsY * 4];
		int pixel = 0;
		for (int i = 0; i < fontPixelsX * fontPixelsY * 4; i += 4, pixel++)
		{
			pImage[i] = 0;
			pImage[i + 1] = 0;
			pImage[i + 2] = 0;
			pImage[i + 3] = 255 * (pPixels[pixel] != 0);
		}
		stbtt_FreeBitmap(pPixels, nullptr);

		pPixels = pImage;
		pImage = nullptr;

		Texture t;

		glCreateTextures(GL_TEXTURE_2D, 1, &t.m_hTexture);
		glBindTexture(GL_TEXTURE_2D, t.m_hTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fontPixelsX, fontPixelsY, 0, GL_RGBA, GL_UNSIGNED_BYTE, pPixels);

		delete[] pPixels;

		vTextures.push_back(std::move(t));
	}

	const stbtt_bakedchar& TextureManager::GetCharInfo(char ch)
	{
		return characters[ch - ' '];
	}

	void TextureManager::Init()
	{
		stbi_set_flip_vertically_on_load(true);
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &iTextureUnitCount);
		vTextures.resize((u32)TextureID::MAX_TEXTURE);

		AddTexture(TextureID::SAND, "textures/Terrain/sand.png");
		AddTexture(TextureID::WATER, "textures/Terrain/water.png");
		AddTexture(TextureID::TOWER_HOVER, "textures/Towers/tower_hover.png");
		AddTexture(TextureID::LAVA, "textures/Terrain/lava.png");
		AddTexture(TextureID::ICE, "textures/Terrain/ice.png");
		AddTexture(TextureID::GRASS, "textures/Terrain/grass.png");
		AddTexture(TextureID::SNOW, "textures/Terrain/snow.png");
		AddTexture(TextureID::FIRE, "textures/Obstacles/fire.png");
		AddTexture(TextureID::ROCK, "textures/Obstacles/rock.png");
		AddTexture(TextureID::TREE, "textures/Obstacles/tree.png");
		AddTexture(TextureID::ICESTONE, "textures/Obstacles/icestone.png");
		AddTexture(TextureID::TOWER_OFF, "textures/Towers/tower_off.png");
		AddTexture(TextureID::TOWER_ALLY, "textures/Towers/tower_ally.png");
		AddTexture(TextureID::TOWER_ENEMY, "textures/Towers/tower_enemy.png");
		AddTexture(TextureID::INFANTRY, "textures/Units/infantry.png");
		AddTexture(TextureID::ARTILLERY, "textures/Units/artillery.png");
		AddTexture(TextureID::SHIELD, "textures/Units/shield.png");
		AddTexture(TextureID::DOGE_COIN, "textures/Others/coin.png");
		AddTexture(TextureID::BACKGROUND, "textures/Others/menu_background.png");
		AddTexture(TextureID::TYPING_PANEL, "textures/Buttons/blue_btn.png");
		AddTexture(TextureID::CONNECT_BUTTON_2, "textures/Buttons/green_btn.png");
		AddTexture(TextureID::UNIT_PANEL, "textures/Others/left_panel.png");
		AddTexture(TextureID::EXIT_BUTTON, "textures/Buttons/exit_btn.png");
		AddTexture(TextureID::HOST_BUTTON, "textures/Buttons/host_btn.png");
		AddTexture(TextureID::CONNECT_BUTTON, "textures/Buttons/connect_btn.png");
		AddTexture(TextureID::END_TURN_BUTTON, "textures/Buttons/end_turn.png");
		AddColor(TextureID::COLOR_GREEN, { 0.0f, 1.0f, 0.0f, 1.0f });
		AddColor(TextureID::COLOR_RED,   { 1.0f, 0.0f, 0.0f, 1.0f });
		AddColor(TextureID::COLOR_BLUE,   { 0.0f, 0.0f, 1.0f, 1.0f });
		AddColor(TextureID::COLOR_TRANSPARENT_GREEN, { 0.0f, 1.0f, 0.0f, 0.3f });
		AddColor(TextureID::COLOR_TRANSPARENT_RED,   { 1.0f, 0.0f, 0.0f, 0.3f });
		AddFont(TextureID::FONT, "textures/Fonts/font.ttf");
	}

	void TextureManager::Shutdown()
	{
		vTextures.clear();
		vTextures.shrink_to_fit();
	}

	void TextureManager::AddTexture(const char* pTextureFilePath)
	{
		vTextures.emplace_back(pTextureFilePath);
	}

	u32 TextureManager::BindTexture(u32 ID)
	{
		for (int i = 0; i < uMaxTexture; i++)
		{
			if (uBoundTextures[i] == ID)
			{
				return i;
			}
		}

		if (uMaxTexture >= iTextureUnitCount)
			return -1;

		glBindTextureUnit(uMaxTexture, TextureManager::vTextures[ID].m_hTexture);
		uBoundTextures[uMaxTexture] = ID;
		uMaxTexture++;

		return uMaxTexture - 1;
	}

	void TextureManager::Flush()
	{
		uMaxTexture = uStaticTextureCount;
	}

}
