#pragma once
#include "RendererCommon.hpp"
#include "App/Window.hpp"
#include "App/Input.hpp"
#include <random>
#include <iostream>
#define MAX_TOWERS 6


namespace gal
{
	inline constexpr u32 mapSize = 30;
	inline constexpr u32 mapTiles = mapSize * mapSize;
	inline glm::vec2 tileSize;
	inline glm::vec2 MapOffset;
	inline float fPixelsPerTile;

	template<typename T>
	T gcd(T a, T b) {
		while (b) {
			int r = a % b;
			a = b;
			b = r;
		}
		return a;
	}

	inline u32 tileSquareDistance(glm::ivec2 a, glm::ivec2 b)
	{
		return std::max<u32>(abs(a.x - b.x), abs(a.y - b.y));
	}

	inline void UpdateMapConstants(glm::ivec2 camera = {0,0})
	{
		const glm::vec2 res = Window::GetResolution();
		float minAxis = std::min(res.y, res.x);
		fPixelsPerTile = minAxis / (float)mapSize;
		tileSize = { fPixelsPerTile, fPixelsPerTile };

		MapOffset.x = (res.x - minAxis) / 2.0f + (camera.x) * (fPixelsPerTile);
		MapOffset.y = (res.y - minAxis) / 2.0f + (camera.y) * (fPixelsPerTile);
	}

	inline glm::ivec2 PixelsToTile(glm::vec2 pos)
	{
		glm::vec2 res = Window::GetResolution();
		glm::ivec2 retval;
		retval = (pos - MapOffset) / fPixelsPerTile;
		return retval;
	}
	inline glm::ivec2 GetHoveringTile()
	{
		glm::vec2 mousePos = Input::GetMousePos();
		mousePos.y = Window::GetResolution().y - mousePos.y;
		glm::ivec2 retval = PixelsToTile(mousePos);
		return retval;
	}

	inline glm::vec2 TileToPixels(glm::vec2 tile)
	{
		glm::vec2 retval;
		retval = (tile * fPixelsPerTile) + MapOffset;
		return retval;
	}

	inline u32 tileDistance(glm::ivec2 tile1, glm::ivec2 tile2)
	{
		return abs(tile1.x - tile2.x) + abs(tile1.y - tile2.y);
	}

	constexpr u64 bit(u64 bitIndex)
	{
		return 1Ui64 << bitIndex;
	}

	inline void perlinNoise1D(i32 iCount, i32 iOctaves, f32 fBias, f32* pOutput, size_t seed = time(0))
	{
		std::mt19937 RandomEngine(seed);
		std::uniform_int_distribution<std::mt19937::result_type> Distribution;

		f32* pSeed = new f32[iCount];

		for (int i = 0; i < iCount; i++)
		{
			pSeed[i] = (f32)Distribution(RandomEngine) / (f32)UINT_MAX; 
		}

		for (i32 i = 0; i < iCount; i++)
		{
			float fNoise = 0.0f;
			float fScale = 1.0f;
			float fScaleAcc = 0.0f;
			for (i32 j = 0; j < iOctaves; j++)
			{
				i32 iPitch = iCount >> j;
				i32 iSample1 = (i / iPitch) * iPitch;
				i32 iSample2 = (iSample1 + iPitch) % iCount;

				f32 fBlend = (f32)(i - iSample1) / (f32)iPitch;
				f32 fSample = (1.0f - fBlend) * pSeed[iSample1] + fBlend * pSeed[iSample2];

				fNoise += fSample * fScale;
				fScaleAcc += fScale;
				fScale /= fBias;
			}

			pOutput[i] = fNoise / fScaleAcc;
		}

		delete[] pSeed;
	}

	inline void perlinNoise2D(i32 iWidth, i32 iHeight, i32 iOctaves, f32 fBias, f32* pOutput, size_t seed = time(0))
	{
		std::random_device r;
//		std::mt19937 RandomEngine(seed);
//		std::uniform_int_distribution<std::mt19937::result_type> Distribution;
		f32* pSeed = new f32[iWidth * iHeight];

		for (int i = 0; i < iWidth * iHeight; i++)
		{
			pSeed[i] = (f32)r() / (f32)UINT_MAX;
	//		pSeed[i] = (f32)Distribution(RandomEngine) / (f32)UINT_MAX;
		}

		for (i32 y = 0; y < iWidth; y++)
			for (i32 x = 0; x < iHeight; x++)
		{
			f32 fNoise = 0.0f;
			f32 fScale = 1.0f;
			f32 fScaleAcc = 0.0f;
			for (i32 o = 0; o < iOctaves; o++)
			{
				i32 iPitch = iWidth >> o;
				i32 iSampleX1 = (x / iPitch) * iPitch;
				i32 iSampleY1 = (y / iPitch) * iPitch;

				i32 iSampleX2 = (iSampleX1 + iPitch) % iWidth;
				i32 iSampleY2 = (iSampleY1 + iPitch) % iHeight;

				f32 fBlendX = (f32)(x - iSampleX1) / (f32)iPitch;
				f32 fBlendY = (f32)(y - iSampleY1) / (f32)iPitch;

				f32 fSampleX = (1.0f - fBlendX) * pSeed[iSampleY1 * iWidth * iSampleX1] + fBlendX * pSeed[iSampleY1 * iWidth + iSampleX2];
				f32 fSampleY = (1.0f - fBlendX) * pSeed[iSampleY2 * iWidth * iSampleX1] + fBlendX * pSeed[iSampleY2 * iWidth + iSampleX2];

				fNoise += (fBlendY * (fSampleY - fSampleX) + fSampleX) * fScale;
				fScaleAcc += fScale;
				fScale /= fBias;
			}

			pOutput[y * iWidth + x] = fNoise / fScaleAcc;
		}

		delete[] pSeed;
	}

}