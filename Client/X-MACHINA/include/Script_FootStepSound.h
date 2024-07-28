#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion



class Script_FootStepSound : public Component {
	COMPONENT(Script_FootStepSound, Component)

private:
	std::array<Transform*, 2>	mFoots{};
	std::array<bool, 2>			mReady{};

	std::vector<std::string> mSounds{};
	int mMaxCnt{};
	int mCurCnt{};

public:
	virtual void Awake() override;
	virtual void Update() override;
};

