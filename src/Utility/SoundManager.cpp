#include "Utility/SoundManager.hpp"

#include <iostream>

void SoundManager::Play(const char* const sound_path, const bool loop) {
	irrklang::ISound* new_sound = Get().m_engine->play2D(sound_path, loop, false, true);
	std::cout << "play " << sound_path << '\n';
	Get().m_soundMap.insert({std::string(sound_path), new_sound});
}

void SoundManager::Stop(const char* const sound_path) {
	irrklang::ISound* sound = Get().m_soundMap.at(std::string(sound_path));
	std::cout << "del " << sound_path << '\n';
	if (sound == nullptr) { return; }
	sound->stop();
	//sound->drop();
}

void SoundManager::StopAll() {
	Get().m_engine->stopAllSounds();
}

SoundManager::SoundManager() {
	m_engine = irrklang::createIrrKlangDevice();
}

SoundManager::~SoundManager() {
	m_engine->drop();
}
