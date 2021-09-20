#include "App/Time.hpp"


namespace gal {

	TimePoint Time::m_tpLast;
	TimePoint Time::m_tpNow;
	Duration Time::m_dDuration;
	float Time::m_fDt = 0.0f;;
	float Time::m_fTimeScale = 1.0f;

	void Time::Init() {
		Time::m_tpLast = std::chrono::high_resolution_clock::now();
	}

	void Time::Step() {
		Time::m_tpNow = std::chrono::high_resolution_clock::now();
		Time::m_dDuration = Time::m_tpNow - Time::m_tpLast;
		Time::m_tpLast = Time::m_tpNow;
		Time::m_fDt = Time::m_dDuration.count() * Time::m_fTimeScale;
	}

	float Time::DeltaTime() {
		return Time::m_fDt;
	}

	void Time::SetTimeScale(float fNewTimeScale) {
		Time::m_fTimeScale = fNewTimeScale;
	}

}