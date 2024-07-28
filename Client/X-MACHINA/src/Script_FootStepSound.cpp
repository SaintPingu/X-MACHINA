#include "stdafx.h"
#include "Script_FootStepSound.h"

#include "SoundMgr.h"

void Script_FootStepSound::Awake()
{
	base::Awake();

	mMaxCnt = 21;
	mSounds.resize(mMaxCnt);
	for (int i = 0; i < 21; ++i) {
		mSounds[i] = "Floor_step" + std::to_string(i);
	}

	mFoots[0] = mObject->FindFrame("Humanoid_ L Toe0");
	mFoots[1] = mObject->FindFrame("Humanoid_ R Toe0");
}

void Script_FootStepSound::Update()
{
	for (int i = 0; i < 2; ++i) {
		float footY = mFoots[i]->GetPosition().y;
		std::cout << footY << std::endl;
		if (!mReady[i]) {
			if (footY > 0.05f) {
				mReady[i] = true;
			}
		}
		else {
			if (footY < 0.02f) {
				mReady[i] = false;
				SoundMgr::I->Play("Env", mSounds[mCurCnt]);
				if (++mCurCnt >= mMaxCnt) {
					mCurCnt = 0;
				}
			}
		}
	}
}
