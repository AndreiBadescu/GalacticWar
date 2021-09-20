#include "Renderer/UI.hpp"
#include "App/Window.hpp"

namespace gal {
	glm::vec2 AnchorPointToPos(UI::Anchor anchorPoint)

	{
		const glm::vec2 res = Window::GetResolution();

		switch (anchorPoint)
		{
		case UI::Anchor::TL: return { 0			   ,0 };
		case UI::Anchor::TM: return { res.x / 2.0f ,0 };
		case UI::Anchor::TR: return { res.x		   ,0 };

		case UI::Anchor::ML: return { 0			  ,res.y / 2.0f };
		case UI::Anchor::MM: return { res.x / 2.0f,res.y / 2.0f };
		case UI::Anchor::MR: return { res.x		  ,res.y / 2.0f };

		case UI::Anchor::BL: return { 0			  ,res.y };
		case UI::Anchor::BM: return { res.x / 2.0f,res.y };
		case UI::Anchor::BR: return { res.x		  ,res.y };
		default: return{ -1,-1 };
		}
	}
	glm::vec2 gal::AnchorPointPivot(UI::Anchor anchorPoint, glm::vec2 v2Pos, glm::vec2 v2Size)
	{
		switch (anchorPoint)
		{
		case UI::Anchor::TL: {
			v2Pos.y -= v2Size.y;
			break;
		}
		case UI::Anchor::TM: {
			v2Pos.y -= v2Size.y;
			v2Pos.x -= v2Size.x / 2.0f;
			break;
		}
		case UI::Anchor::TR: {
			v2Pos.y -= v2Size.y;
			v2Pos.x -= v2Size.x;
			break;
		}

		case UI::Anchor::ML: {
			v2Pos.y -= v2Size.y / 2.0f;
			break;
		}
		case UI::Anchor::MM: {
			v2Pos.y -= v2Size.y / 2.0f;
			v2Pos.x -= v2Size.x / 2.0f;
			break;
		}
		case UI::Anchor::MR: {
			v2Pos.y -= v2Size.y / 2.0f;
			v2Pos.x -= v2Size.x;
			break;
		}

		case UI::Anchor::BL: {
			break;
		}
		case UI::Anchor::BM: {
			v2Pos.x -= v2Size.x / 2.0f;
			break;
		}
		case UI::Anchor::BR: {
			v2Pos.x -= v2Size.x;
			break;
		}
		}

		return v2Pos;
	}

	glm::vec2 UItoNormalPos(UI::Anchor anchorPoint, glm::vec2 v2Pos, glm::vec2 v2Size)
	{
		const glm::vec2 res = Window::GetResolution();
		glm::vec2 retval;
		retval = AnchorPointToPos(anchorPoint) + v2Pos;
		retval.y = res.y - retval.y;
		retval = AnchorPointPivot(anchorPoint, retval, v2Size);
		return retval;
	}

}