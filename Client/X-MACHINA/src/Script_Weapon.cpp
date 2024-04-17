#include "stdafx.h"
#include "Script_Weapon.h"

#include "Script_Bullet.h"
#include "Script_Player.h"

#include "Component/Rigidbody.h"

#include "Scene.h"
#include "Object.h"
#include "ObjectPool.h"
#include "Timer.h"





#pragma region Script_Weapon
void Script_Weapon::Awake()
{
	base::Awake();

	mMuzzle = mObject->FindFrame("FirePos");
	InitValues();
	CreateBulletPool();
}

void Script_Weapon::Start()
{
	base::Start();

	mCurMag = mMaxMag;
}

void Script_Weapon::Update()
{
	if (mOwner->IsInDraw() || mOwner->IsInPutBack() || mIsReload) {
		return;
	}

	updateFunc();
}

void Script_Weapon::FireBullet()
{
	mOwner->BulletFired();
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

void Script_Weapon::InitReload()
{
	mCurMag = 0;
	mCurReloadTime = 0.f;
	mIsReload = true;
}

void Script_Weapon::EndReload()
{
	mCurMag = mMaxMag;
	mCurReloadTime = 0.f;
	mIsReload = false;
}

void Script_Weapon::StartReload()
{
	InitReload();

	if (mOwner) {
		mOwner->StartReload();
	}
}

void Script_Weapon::Update_SemiAuto()
{
	if (CanFire()) {
		if (mIsShooting && !mIsBeforeShooting) {
			Fire();
			mIsBeforeShooting = true;
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
			Fire();
		}
	}
	else {
		mCurFireDelay += DeltaTime();
	}
}

void Script_Weapon::Fire()
{
	mCurFireDelay = 0.f;
	FireBullet();

	if (--mCurMag <= 0) {
		mIsReload = true;
		StartReload();
	}
}
#pragma endregion

















#pragma region Script_BulletWeapon
void Script_BulletWeapon::FireBullet()
{
	base::FireBullet();

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
	mBulletPool = Scene::I->CreateObjectPool("bullet", mMaxMag, std::bind(&Script_BulletWeapon::BulletInitFunc, this, std::placeholders::_1));
}
#pragma endregion