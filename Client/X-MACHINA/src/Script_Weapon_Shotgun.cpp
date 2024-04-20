#include "stdafx.h"
#include "Script_Weapon_Shotgun.h"

#include "Script_Bullet.h"

#include "Object.h"
#include "ObjectPool.h"

void Script_Weapon_Shotgun::Awake()
{
	base::Awake();

	SetFiringMode(FiringMode::SemiAuto);
}

void Script_Weapon_Shotgun::FireBullet()
{
	Script_Weapon::FireBullet();	// Script_BulletWeapon�� ���� �Ѿ� �߻��̱� ������ �����Ѵ�.

	const auto& bullets = mBulletPool->GetMulti(mBulletCntPerShot, true);
	for (auto& bullet : bullets) {
		auto& bulletScript = bullet->GetComponent<Script_Bullet>();

		Vec2 err = Vec2(Math::RandFloat(mErrX.x, mErrX.y), Math::RandFloat(mErrY.x, mErrY.y));
		const float bulletSpeedErr = Math::RandFloat(0, mSpeerErr);

		bulletScript->SetSpeed(mkBulletSpeed - bulletSpeedErr);
		bulletScript->Fire(*mMuzzle, err);
	}
}






void Script_Weapon_DBMS::InitValues()
{
	mMaxFireDelay     = CalcFireDelay(mkRPM);
	mMaxReloadTime    = 2.2f;
	mMaxDistance      = 8.f;
	mBulletCntPerMag  = 2;
	mMaxMag           = 10;
	mBulletCntPerShot = mkBulletCntPerShot;

	mSpeerErr = 20.f;
	mErrX = Vec2(-20.f, 20.f);
	mErrY = Vec2(-10.f, 6.f);
}

void Script_Weapon_DBMS::BulletInitFunc(rsptr<InstObject> bullet) const
{
	base::InitBullet(bullet, mkBulletDamage, mkBulletSpeed);
}