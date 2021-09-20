#pragma once
#include "RendererCommon.hpp"


namespace gal
{

	enum class TextureID
	{
		SAND,
		WATER,
		LAVA,
		ICE,
		GRASS,
		SNOW,
		FIRE,
		ROCK,
		TREE,
		ICESTONE,
		TOWER_OFF,
		TOWER_ALLY,
		TOWER_ENEMY,
		TOWER_HOVER,
		INFANTRY,
		ARTILLERY,
		SHIELD,
		COLOR_GREEN,
		COLOR_RED,
		COLOR_BLUE,
		COLOR_TRANSPARENT_GREEN,
		COLOR_TRANSPARENT_RED,
		CONNECT_BUTTON_2,
		TYPING_PANEL,
		DOGE_COIN,
		BACKGROUND,
		UNIT_PANEL,
		EXIT_BUTTON,
		END_TURN_BUTTON,
		CONNECT_BUTTON,
		HOST_BUTTON,
		FONT,
		MAX_TEXTURE
	};

	class Texture
	{
	public:
		Texture();
		Texture(const char* pImageFilePath);
		Texture(Texture&& other) noexcept;
		Texture(const Texture& other) = delete;
		Texture& operator=(Texture&& other) noexcept;
		~Texture();


		void Bind(u32 uTextureUnit);
		static void UnbindTextureUnit(u32 uTextureUnit);

	private:
		friend class TextureManager;

		handle_t m_hTexture;

	};


}
