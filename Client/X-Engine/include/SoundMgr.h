#pragma once

// has Channel
#define SOUND_LIST_BGM 7
#define SOUND_LIST_Effect 10
#define SOUND_LIST_Skill 3
#define SOUND_LIST_BOSS 6
#define SOUND_LIST_SELECT 4
#define SOUND_CHANNEL_LIST 4

// has no Channel
#define SOUND_LIST_Hit 4


enum class BGMSound { Intro = 0, };
enum class SoundChannel { Bgm = 0, };


struct FMOD_SYSTEM;
struct FMOD_SOUND;
struct FMOD_CHANNEL;


class SoundMgr : public Singleton<SoundMgr> {
	friend Singleton;

private:
	// Stop은 Play후에 사용할 것
	FMOD_SYSTEM* mSoundSystem = nullptr;
	std::unordered_map<std::string, FMOD_SOUND*> mSoundList;
	std::unordered_map<std::string, FMOD_CHANNEL*> mChannels;

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