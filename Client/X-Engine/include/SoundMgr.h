#pragma once

namespace FMOD {
	class System;
	class Sound;
	class Channel;
}

class SoundMgr : public Singleton<SoundMgr> {
	friend Singleton;

private:
	// Stop은 Play후에 사용할 것
	FMOD::System* mSoundSystem = nullptr;
	std::unordered_map<std::string, FMOD::Sound*> mSoundList;
	std::unordered_map<std::string, FMOD::Channel*> mChannels;

public:
	void Init();
	void Release();
	void LoadSounds();

	void Update();

public:
	void Play(const std::string& channelName, const std::string& soundName, float volume = 1.0f, bool isStopBefore = false);
	void PlayNoChannel(const std::string& channelName, const std::string& soundName);
	void Stop(const std::string& channelName);

private:
	bool CheckChannelName(const std::string& channelName);
	bool CheckSoundName(const std::string& soundPath);
};