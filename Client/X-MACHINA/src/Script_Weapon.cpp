#include "stdafx.h"
#include "Script_Weapon.h"

#include "Timer.h"

void Script_Weapon::Awake()
{
	mMuzzle = mObject->FindFrame("FirePos");
	CreateBulletPool();
}

void Script_Weapon::Update()
{
	if (mCurFireDelay >= mMaxFireDelay) {
		if (mIsShooting) {
			mCurFireDelay = 0.f;
			FireBullet();
		}
	}
	else {
		mCurFireDelay += DeltaTime();
	}
}
