#include "RendererCommon.hpp"
#include "Renderer/Texture.hpp"
#include "deps/stb_image/stb_image.h"
#include <stdexcept>
#include <iostream>

namespace gal
{
	Texture::Texture()
		:m_hTexture(nullhandle)
	{
	}
	Texture::Texture(const char* t_pImageFilePath)
	{

		glCreateTextures(GL_TEXTURE_2D, 1, &m_hTexture);
		glBindTexture(GL_TEXTURE_2D, m_hTexture);

		i32 iImageSizeX;
		i32 iImageSizeY;
		i32 iChannels;
		u8* pImage = (u8*)stbi_load(t_pImageFilePath, &iImageSizeX, &iImageSizeY, &iChannels, 0);

		if (pImage){



			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			if(iChannels == 4)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iImageSizeX, iImageSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
			else if(iChannels == 3)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iImageSizeX, iImageSizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, pImage);
			else if (iChannels == 2)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iImageSizeX, iImageSizeY, 0, GL_RG, GL_UNSIGNED_BYTE, pImage);
			else if (iChannels == 1)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iImageSizeX, iImageSizeY, 0, GL_R, GL_UNSIGNED_BYTE, pImage);

			stbi_image_free(pImage);
		} else{
			std::cerr << t_pImageFilePath << '\n';
			throw std::runtime_error("Could not read image from file");
		}

	}

	Texture::Texture(Texture&& other) noexcept
		:m_hTexture(other.m_hTexture)
	{
		other.m_hTexture = nullhandle;
	}

	Texture& Texture::operator=(Texture&& other) noexcept
	{
		m_hTexture = other.m_hTexture;
		other.m_hTexture = nullhandle;
		return *this;
	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &m_hTexture);
	}

	void Texture::Bind(u32 uTextureUnit)
	{
		glBindTextureUnit(uTextureUnit, m_hTexture);
	}

	void Texture::UnbindTextureUnit(u32 uTextureUnit)
	{
		glBindTexture(uTextureUnit, nullhandle);
	}
}