#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include "deps/stb_truetype/stb_truetype.h"
#include "Renderer/BatchRenderer.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/UI.hpp"
#include "Utility/TextureManager.hpp"
#include "Utility/Math.hpp"
#include "App/Window.hpp"

namespace gal {

	global_variable constexpr u64 uMaxBatchSize = 10000;

	struct Vertex
	{
		Vertex(glm::vec2 vPos, glm::vec2 vTex, float uTextureID)
			:vPos(vPos),
			vTex(vTex),
			fTexID(uTextureID)
		{}
		glm::vec2 vPos;
		glm::vec2 vTex;
		float fTexID;
	};

	enum BufferIndex
	{
		BUFFER_INDEX_VBO,
		BUFFER_INDEX_IBO
	};

	struct BatchRendererData
	{
		handle_t buffers[2];
		handle_t vao;
		Shader shader;
		glm::uvec2 res;
		u64 uStaticOffset;

		std::vector<Vertex> vVertices;

	};

	global_variable BatchRendererData* pRendererData = nullptr;





	void BatchRenderer::Init()
	{

		assert(pRendererData == nullptr && "Batch Renderer already initialized");
		pRendererData = new BatchRendererData();

		pRendererData->res = { 0,0 };

		pRendererData->shader = Shader("shaders/vertex.shader", "shaders/fragment.shader");

		pRendererData->shader.Bind();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		int samplers[32];
		for (int i = 0; i < 32; i++)
		{
			samplers[i] = i;
		}

		GLint samplerLoc = glGetUniformLocation(pRendererData->shader.GetProgramHandle(), "fTextures");
		glUniform1iv(samplerLoc, 32, samplers);

		glCreateVertexArrays(1, &pRendererData->vao);
		glBindVertexArray(pRendererData->vao);

		glCreateBuffers(2, pRendererData->buffers);

		glBindBuffer(GL_ARRAY_BUFFER, pRendererData->buffers[BUFFER_INDEX_VBO]);
		glBufferData(GL_ARRAY_BUFFER, uMaxBatchSize * 4 * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, vPos));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, vTex));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, fTexID));


		u32* indices = new u32[uMaxBatchSize * 6];
		u32 offset = 0;
		for (int i = 0; i < uMaxBatchSize * 6; i += 6, offset += 4)
		{
			indices[i + 0] = offset + 0;
			indices[i + 1] = offset + 1;
			indices[i + 2] = offset + 2;

			indices[i + 3] = offset + 2;
			indices[i + 4] = offset + 3;
			indices[i + 5] = offset + 0;
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pRendererData->buffers[BUFFER_INDEX_IBO]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, uMaxBatchSize * 6 * sizeof(u32), indices, GL_STATIC_DRAW);

		delete[] indices;
	}

	void BatchRenderer::BeginScene()
	{
		if (Window::GetResolution() != pRendererData->res)
		{
			pRendererData->res = Window::GetResolution();
			auto loc = glGetUniformLocation(pRendererData->shader.GetProgramHandle(), "uResolution");
			glUniform2f(loc, Window::GetResolution().x, Window::GetResolution().y);
		}
	}

	void BatchRenderer::AddStaticQuad(glm::vec2 v2Pos, glm::vec2 v2Size, u32 uTextureID)
	{
		u32 texIndex = TextureManager::BindStaticTexture(uTextureID);

		pRendererData->vVertices.emplace_back(glm::vec2(v2Pos.x - v2Size.x / 2.0f, v2Pos.y), glm::vec2(0, 0), (float)texIndex);
		pRendererData->vVertices.emplace_back(glm::vec2(v2Pos.x + v2Size.x / 2.0f, v2Pos.y), glm::vec2(1, 0), (float)texIndex);
		pRendererData->vVertices.emplace_back(glm::vec2(v2Pos.x + v2Size.x / 2.0f, v2Pos.y + v2Size.y), glm::vec2(1, 1), (float)texIndex);
		pRendererData->vVertices.emplace_back(glm::vec2(v2Pos.x - v2Size.x / 2.0f, v2Pos.y + v2Size.y), glm::vec2(0, 1), (float)texIndex);

		pRendererData->uStaticOffset += 4 * sizeof(Vertex);
	}

	void BatchRenderer::ClearStaticQuads()
	{
		pRendererData->uStaticOffset = 0;
	}

	void BatchRenderer::UploadStaticQuads()
	{
		glBufferSubData(GL_ARRAY_BUFFER, 0, pRendererData->vVertices.size() * sizeof(Vertex), pRendererData->vVertices.data());
		pRendererData->vVertices.clear();
	}

	void BatchRenderer::UpdateCamera(glm::vec2 newCam)
	{
		auto loc = glGetUniformLocation(pRendererData->shader.GetProgramHandle(), "uCam");
		glUniform2f(loc, newCam.x, newCam.y);
	}

	internal_func glm::vec4 asciiToGlyphCoords(char c)
	{
		unsigned int index = c - ' ';

		const stbtt_bakedchar& charInfo = TextureManager::GetCharInfo(c);


		float x0 = charInfo.x0 / (float)TextureManager::fontPixelsX;
		float x1 = charInfo.x1 / (float)TextureManager::fontPixelsX;
		float y0 = charInfo.y0 / (float)TextureManager::fontPixelsY;
		float y1 = charInfo.y1 / (float)TextureManager::fontPixelsY;

		return { x0,y0,x1,y1 };
	}

	void BatchRenderer::DrawText(glm::vec2 v2Pos, float fontSize, const char* pText)
	{
		u32 texIndex = TextureManager::BindTexture((u32)TextureID::FONT);
		if (texIndex == -1)
		{
			BatchRenderer::Flush();
			texIndex = TextureManager::BindTexture((u32)TextureID::FONT);
		}

		int len = strlen(pText);
		float advance = 0;
		u64 shiftStartIndex = pRendererData->vVertices.size();
		for (int i = 0; i < len; i++)
		{
			const stbtt_bakedchar& charInfo = TextureManager::GetCharInfo(toupper(pText[i]));
			glm::vec4 glyphCoords = asciiToGlyphCoords(toupper(pText[i]));
			float thisAdvance = 0;
			if (pText[i] == '.')
				thisAdvance = (fontSize / 100.0f) * TextureManager::GetCharInfo(toupper(pText[i])).xadvance / 5.0f;
			else
				thisAdvance = (fontSize / 100.0f) * TextureManager::GetCharInfo(toupper(pText[i])).xadvance;

			pRendererData->vVertices.emplace_back(glm::vec2(v2Pos.x + advance			   , v2Pos.y), glm::vec2(glyphCoords.x, glyphCoords.w), (float)texIndex);
			pRendererData->vVertices.emplace_back(glm::vec2(v2Pos.x + advance + thisAdvance, v2Pos.y), glm::vec2(glyphCoords.z, glyphCoords.w), (float)texIndex);
			pRendererData->vVertices.emplace_back(glm::vec2(v2Pos.x + advance + thisAdvance, v2Pos.y + (fontSize / 80.0f) * (charInfo.y1 - charInfo.y0)), glm::vec2(glyphCoords.z, glyphCoords.y), (float)texIndex);
			pRendererData->vVertices.emplace_back(glm::vec2(v2Pos.x + advance			   , v2Pos.y + (fontSize / 80.0f) * (charInfo.y1 - charInfo.y0)), glm::vec2(glyphCoords.x, glyphCoords.y), (float)texIndex);

			advance += thisAdvance + 5;

		}
		u64 shiftEndIndex = pRendererData->vVertices.size();


		for (u64 i = shiftStartIndex; i < shiftEndIndex; i++)
		{
			pRendererData->vVertices[i].vPos.x -= advance / 2.0f;
		}

	}


	void BatchRenderer::DrawQuad(glm::vec2 v2Pos, glm::vec2 v2Size, u32 uTextureID)
	{
		u32 texIndex = TextureManager::BindTexture(uTextureID);
		if (texIndex == -1)
		{
			BatchRenderer::Flush();
			texIndex = TextureManager::BindTexture(uTextureID);
		}
		pRendererData->vVertices.emplace_back(glm::vec2(v2Pos.x, v2Pos.y), glm::vec2(0, 0), (float)texIndex);
		pRendererData->vVertices.emplace_back(glm::vec2(v2Pos.x + v2Size.x, v2Pos.y), glm::vec2(1, 0), (float)texIndex);
		pRendererData->vVertices.emplace_back(glm::vec2(v2Pos.x + v2Size.x, v2Pos.y + v2Size.y), glm::vec2(1, 1), (float)texIndex);
		pRendererData->vVertices.emplace_back(glm::vec2(v2Pos.x, v2Pos.y + v2Size.y), glm::vec2(0, 1), (float)texIndex);

		if (pRendererData->vVertices.size() / 4 >= uMaxBatchSize)
		{
			BatchRenderer::Flush();
		}
	}

	void BatchRenderer::DrawQuad(glm::vec2 v2Pos, glm::vec2 v2Size, TextureID tID)
	{
		DrawQuad(v2Pos, v2Size, (u32)tID);
	}

	void BatchRenderer::DrawTile(glm::uvec2 v2Tile, glm::vec2 v2Size, u32 uTextureID)
	{
		glm::vec2 v2Pos = TileToPixels(v2Tile);
		v2Pos.y = Window::GetResolution().y - v2Pos.y - fPixelsPerTile;
		DrawQuad(v2Pos, v2Size, uTextureID);
	}

	void BatchRenderer::DrawTile(glm::uvec2 v2Tile, glm::vec2 v2Size, TextureID tID)
	{
		DrawTile(v2Tile, v2Size, (u32)tID);
	}

	void BatchRenderer::DrawUI(UI::Anchor anchorPoint, glm::vec2 v2Pos, glm::vec2 v2Size, TextureID tId)
	{
		DrawUI(anchorPoint, v2Pos, v2Size, (u32)tId);
	}

	void BatchRenderer::DrawUI(UI::Anchor anchorPoint, glm::vec2 v2Pos, glm::vec2 v2Size, u32 uTextureID)
	{
		const glm::vec2 res = Window::GetResolution();
		v2Pos = UItoNormalPos(anchorPoint, v2Pos, v2Size);
		//glm::vec2 anchorOffset = AnchorPointToPos(anchorPoint);
		//v2Pos += anchorOffset;
		//v2Pos.y = res.y - v2Pos.y;
		//v2Pos = AnchorPointPivot(anchorPoint, v2Pos, v2Size);
		DrawQuad(v2Pos, v2Size, uTextureID);
	}

	void BatchRenderer::EndScene()
	{
		if (pRendererData->vVertices.size() > 0)
			BatchRenderer::Flush();
	}

	void BatchRenderer::Flush()
	{
		glBufferSubData(GL_ARRAY_BUFFER, pRendererData->uStaticOffset, pRendererData->vVertices.size() * sizeof(Vertex), pRendererData->vVertices.data());
		glDrawElements(GL_TRIANGLES, 3 * (pRendererData->vVertices.size() + pRendererData->uStaticOffset) / 2, GL_UNSIGNED_INT, nullptr);
		pRendererData->vVertices.clear();
		TextureManager::Flush();
	}

}