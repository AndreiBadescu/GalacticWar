#pragma once
#include "RendererCommon.hpp"
#include "Texture.hpp"
#include <string>
namespace gal {
	struct UI
	{
		//R = right
		//L = left
		//M = middle
		//T = top
		//B = bottom

		std::string text;
		glm::vec2 pos;
		glm::vec2 size;
		enum class Type : u32
		{
			BUTTON,
			IMAGE,
			TEXT_INPUT
		}uiType;
		TextureID textureID;
		union
		{
			struct
			{
				TextureID hoverTextureID;
				void(*pButtonFunc)(void*);
			}buttonData;
			struct
			{
				const char* pBackgroundText;
			};
		};

		enum class Anchor : u8
		{
			TL, TM, TR,
			ML, MM, MR,
			BL, BM, BR
		}anchorPoint;
	};


	glm::vec2 AnchorPointToPos(UI::Anchor anchorPoint);
	glm::vec2 AnchorPointPivot(UI::Anchor anchorPoint, glm::vec2 v2Pos, glm::vec2 v2Size);
	glm::vec2 UItoNormalPos(UI::Anchor anchorPoint, glm::vec2 v2Pos, glm::vec2 v2Size);

}