#include "EnginePch.h"
#include "SoundMgr.h"

#include "FileIO.h"
#include "ResourceMgr.h"

inline constexpr void CheckResult(const FMOD_RESULT& result)
{
	if (result != FMOD_OK)
	{
		assert(0);
	}
}


void SoundMgr::Init()
{
	Release();
	FMOD_System_Create(&mSoundSystem, FMOD_VERSION);
	FMOD_System_Init(mSoundSystem, 32, FMOD_INIT_NORMAL, NULL);
}

void SoundMgr::Release()
{
	mSoundList.clear();
	mChannels.clear();
	if (mSoundSystem) {
		FMOD_System_Close(mSoundSystem);
		FMOD_System_Release(mSoundSystem);
		mSoundSystem = nullptr;
	}
}

void SoundMgr::LoadSounds()
{
	Release();
	Init();

	const std::string rootFolder = "Import/Sounds/";

	constexpr int channelCnt = 2;

	for (const auto& folder : std::filesystem::directory_iterator(rootFolder)) {
		std::string channelName = folder.path().filename().string();
		const std::string folder = rootFolder + channelName + "/";
		char loopTypeName = channelName.front();
		channelName.erase(0, 2);

		int loopType{};
		switch (loopTypeName) {
		case 'L':
			loopType = FMOD_LOOP_NORMAL;
			break;
		case 'N':
			loopType = FMOD_DEFAULT;
			break;
		default:
			assert(0);
			break;
		}

		for (const auto& file : std::filesystem::directory_iterator(folder)) {
			const std::string fileName = file.path().filename().string();
			const std::string filePath = folder + fileName;
			const std::string soundName = FileIO::RemoveExtension(fileName);
			mChannels[channelName] = nullptr;
			CheckResult(FMOD_System_CreateSound(mSoundSystem, filePath.data(), loopType, 0, &mSoundList[channelName + "/" + soundName]));
		}
	}
}

void SoundMgr::Update()
{
	FMOD_System_Update(mSoundSystem);
}

void SoundMgr::Play(const std::string& channelName, const std::string& soundName, float volume, bool isStopBefore)
{
	if (!CheckChannelName(channelName)) {
		return;
	}
	const std::string soundPath = channelName + "/" + soundName;
	if (!CheckSoundName(soundPath)) {
		return;
	}

	auto& sound = mSoundList[soundPath];
	auto& channel = mChannels[channelName];

	if (isStopBefore && channel) {
		FMOD_Channel_Stop(channel);
	}

	FMOD_System_PlaySound(mSoundSystem, sound, 0, false, &channel);
	FMOD_Channel_SetVolume(channel, volume);
}

void SoundMgr::PlayNoChannel(const std::string& channelName, const std::string& soundName)
{
	if (!CheckChannelName(channelName)) {
		return;
	}
	const std::string soundPath = channelName + "/" + soundName;
	if (!CheckSoundName(soundPath)) {
		return;
	}

	auto& sound = mSoundList[soundPath];
	FMOD_System_PlaySound(mSoundSystem, sound, 0, false, nullptr);
}

void SoundMgr::Stop(const std::string& channelName)
{
	if (!CheckChannelName(channelName)) {
		return;
	}

	auto& channel = mChannels[channelName];
	FMOD_Channel_Stop(channel);
}

bool SoundMgr::CheckChannelName(const std::string& channelName)
{
	if (!mChannels.count(channelName)) {
		std::cout << "[WARNING] Channel does not exist : " << channelName << std::endl;
		return false;
	}

	return true;
}

bool SoundMgr::CheckSoundName(const std::string& soundPath)
{
	if (!mSoundList.count(soundPath)) {
		std::cout << "[WARNING] Sound does not exist : " << soundPath << std::endl;
		return false;
	}

	return true;
}
