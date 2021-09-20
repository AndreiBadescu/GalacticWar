#pragma once

#include <Irrklang/irrKlang.h>
#include <unordered_map>
#include <string>

class SoundManager {
public:
	static void Play(const char* const sound_path, const bool loop = false);
	static void Stop(const char* const sound_path);
	static void StopAll();

private:
	irrklang::ISoundEngine* m_engine;
	std::unordered_map<std::string, irrklang::ISound*> m_soundMap;

	SoundManager();
	SoundManager(const SoundManager&) = delete;
	SoundManager& operator=(const SoundManager&) = delete;
	~SoundManager();

	static SoundManager& Get() {
		static SoundManager unique_instance;
		return unique_instance;
	}
};

