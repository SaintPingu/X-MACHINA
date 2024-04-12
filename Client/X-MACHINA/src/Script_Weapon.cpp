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
	updateFunc();
}

void Script_Weapon::SetFiringMode(FiringMode firingMode)
{
	switch (firingMode) {
	case FiringMode::SemiAuto:
	case FiringMode::BoltAction:
		updateFunc = std::bind(&Script_Weapon::Update_SemiAuto, this);
		break;
	case FiringMode::Auto:
		updateFunc = std::bind(&Script_Weapon::Update_Auto, this);
		break;
	default:
		assert(0);
		break;
	}
}

void Script_Weapon::Update_SemiAuto()
{
	if (CanFire()) {
		if (mIsShooting && !mIsBeforeShooting) {
			mCurFireDelay = 0.f;
			mIsBeforeShooting = true;
			FireBullet();
		}
	}
	else {
		mCurFireDelay += DeltaTime();
	}
}

void Script_Weapon::Update_Auto()
{
	if (CanFire()) {
		if (mIsShooting) {
			mCurFireDelay = 0.f;
			FireBullet();
		}
	}
	else {
		mCurFireDelay += DeltaTime();
	}
}
