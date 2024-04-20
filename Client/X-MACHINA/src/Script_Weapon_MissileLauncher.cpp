#include "stdafx.h"
#include "Script_Weapon_MissileLauncher.h"

#include "Scene.h"
#include "Object.h"
#include "ObjectPool.h"

void Script_Weapon_MissileLauncher::Awake()
{
	base::Awake();

	SetFiringMode(FiringMode::SemiAuto);
}

void Script_Weapon_MissileLauncher::CreateBulletPool()
{
	mBulletPool = Scene::I->CreateObjectPool("SM_Missile", mBulletCntPerMag * mBulletCntPerShot, std::bind(&Script_Weapon_MissileLauncher::BulletInitFunc, this, std::placeholders::_1));
}






void Script_Weapon_Burnout::FireBullet()
{
	base::FireBullet();

	if (mCurBulletCnt > 0) {
		mMuzzle = mMuzzles[mCurBulletCnt - 1];
	}
	else {
		mMuzzle = mMuzzles[mBulletCntPerMag - 1];
	}
}

void Script_Weapon_Burnout::InitValues()
{
	mMaxFireDelay     = 0.1f;	 // �߻� ������ ���� ����
	mMaxReloadTime    = 5.9f;
	mMaxDistance      = 43.f;
	mBulletCntPerMag  = mkBulletCntPerMag;
	mMaxMag           = 3;

	mErrX = Vec2(-2.f, 2.f);
	mErrY = Vec2(-2.f, 2.f);

	mMuzzles.resize(mBulletCntPerMag);
	mMuzzles[mBulletCntPerMag - 1] = mObject->FindFrame("FirePos_1");
	mMuzzles[mBulletCntPerMag - 2] = mObject->FindFrame("FirePos_2");
	mMuzzles[mBulletCntPerMag - 3] = mObject->FindFrame("FirePos_3");
	mMuzzles[mBulletCntPerMag - 4] = mObject->FindFrame("FirePos_4");
	mMuzzle = mMuzzles[mBulletCntPerMag - 1];
}

void Script_Weapon_Burnout::BulletInitFunc(rsptr<InstObject> bullet) const
{
	base::InitBullet(bullet, mkBulletDamage, mkBulletSpeed);
}