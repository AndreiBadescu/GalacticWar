#pragma once
#include "RendererCommon.hpp"
#include <chrono>


namespace gal {

#ifdef __clang__
	using TimePoint = std::chrono::_V2::system_clock::time_point;
	using Duration = std::chrono::duration<float>;
#else
	using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;
	using Duration = std::chrono::duration<float>;
#endif

	class Time {
	public:
		static void Init();
		static void Step();
		static float DeltaTime();
		static void SetTimeScale(float fNewTimeScale);

	private:
		static float m_fDt;
		static float m_fTimeScale;
		static TimePoint m_tpLast, m_tpNow;
		static Duration m_dDuration;
	};
}