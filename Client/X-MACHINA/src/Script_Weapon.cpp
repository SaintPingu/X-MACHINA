#include "stdafx.h"
#include "Script_Weapon.h"

#include "Script_Bullet.h"

#include "Component/Rigidbody.h"

#include "Scene.h"
#include "Object.h"
#include "ObjectPool.h"
#include "Timer.h"





#pragma region Script_Weapon
void Script_Weapon::Awake()
{
	mMuzzle = mObject->FindFrame("FirePos");
	InitValues();
	CreateBulletPool();
}

void Script_Weapon::Start()
{
	mCurMag = mMaxMag;
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
#pragma endregion

















#pragma region Script_BulletWeapon
void Script_BulletWeapon::FireBullet()
{
	auto& bullet = mBulletPool->Get(true);
	if (bullet) {
		auto& bulletScript = bullet->GetComponent<Script_Bullet>();
		bulletScript->Fire(*mMuzzle);
	}
}

void Script_BulletWeapon::InitBullet(rsptr<InstObject> bullet, float damage, float speed) const
{
	bullet->SetTag(ObjectTag::Bullet);

	auto& bulletScript = bullet->AddComponent<Script_Bullet>();
	bulletScript->SetDamage(damage);
	bulletScript->SetSpeed(speed);

	bullet->AddComponent<Rigidbody>();
}


void Script_BulletWeapon::CreateBulletPool()
{
	mBulletPool = scene->CreateObjectPool("bullet", mMaxMag, std::bind(&Script_BulletWeapon::BulletInitFunc, this, std::placeholders::_1));
}
#pragma endregion