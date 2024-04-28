#include "stdafx.h"
#include "Script_Weapon.h"

#include "Script_Bullet.h"
#include "Script_Missile.h"
#include "Script_Player.h"

#include "Component/Rigidbody.h"
#include "Component/ParticleSystem.h"

#include "Scene.h"
#include "Object.h"
#include "ObjectPool.h"
#include "Timer.h"





#pragma region Script_Weapon
void Script_Weapon::Awake()
{
	base::Awake();

	mMuzzle = mObject->FindFrame("FirePos");

	SetParticleSystemNames();

	InitValues();
	CreateBulletPool();
}

void Script_Weapon::Start()
{
	base::Start();

	mCurMag = mMaxMag;
	mCurBulletCnt = mBulletCntPerMag;
	mCurFireDelay = mMaxFireDelay;
}

void Script_Weapon::Update()
{
	if (mOwner->IsInDraw() || mOwner->IsInPutBack() || mIsReload || mCurBulletCnt <= 0) {
		return;
	}

	if (CanFire()) {
		updateFunc();
	}
	else {
		mCurFireDelay += DeltaTime();
	}
}

void Script_Weapon::FireBullet()
{
	--mCurBulletCnt;
	ParticleManager::I->Play("WFX_Muzzle_Flash", mMuzzle);
	ParticleManager::I->Play("WFX_Muzzle_Smoke", mMuzzle);

	mOwner->BulletFired();
}

void Script_Weapon::SetFiringMode(FiringMode firingMode)
{
	switch (firingMode) {
	case FiringMode::Auto:
		updateFunc = std::bind(&Script_Weapon::Update_Auto, this);
		break;
	case FiringMode::SemiAuto:
		updateFunc = std::bind(&Script_Weapon::Update_SemiAuto, this);
		break;
	case FiringMode::BoltAction:
		updateFunc = std::bind(&Script_Weapon::Update_BoltAction, this);
		break;
	default:
		assert(0);
		break;
	}
}

bool Script_Weapon::CheckReload()
{
	if (mCurMag <= 0) {	// 모든 탄창 소진
		return false;
	}

	StartReload();
	
	return true;
}

void Script_Weapon::EndReload()
{
	--mCurMag;
	mCurBulletCnt = mBulletCntPerMag;
	mCurReloadTime = 0.f;
	mCurFireDelay = mMaxFireDelay;
	mIsReload = false;
}

bool Script_Weapon::InitReload()
{
	mCurBulletCnt = mCurBulletCnt > 0 ? 1 : 0;  // 약실 -> 1발 장전 상태
	mCurReloadTime = 0.f;
	mIsReload = true;

	return true;
}

void Script_Weapon::StartReload()
{
	InitReload();

	if (mOwner) {
		mOwner->StartReload();
	}
}

void Script_Weapon::Update_Auto()
{
	if (mIsShooting) {
		Fire();
	}
}

void Script_Weapon::Update_SemiAuto()
{
	if (mIsShooting && !mIsBeforeShooting) {
		Fire();
		mIsBeforeShooting = true;
	}
}

void Script_Weapon::Update_BoltAction()
{
	if (mIsShooting && !mIsBeforeShooting) {
		Fire();
		mIsBeforeShooting = true;

		if (mCurBulletCnt > 0 && mOwner) {
			mIsBoltAction = true;
			mOwner->BoltAction();
		}
	}
	else {
		mIsBoltAction = false;
	}
}


void Script_Weapon::Fire()
{
	if (mCurBulletCnt <= 0) {
		return;
	}

	mCurFireDelay = 0.f;
	FireBullet();

	if (mCurBulletCnt <= 0) {
		CheckReload();
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

		Vec2 err = Vec2(Math::RandFloat(mErrX.x, mErrX.y), Math::RandFloat(mErrY.x, mErrY.y));
		const float bulletSpeedErr = Math::RandFloat(0, mSpeerErr);

		bulletScript->SetSpeed(GetBulletSpeed() - bulletSpeedErr);

		bulletScript->Fire(*mMuzzle, err);
	}
}

void Script_BulletWeapon::InitBullet(rsptr<InstObject> bullet, float damage, float speed, BulletType bulletType) const
{
	bullet->AddComponent<Rigidbody>();

	sptr<Script_Bullet> bulletScript{};
	switch (bulletType) {
	case BulletType::Bullet:
		bulletScript = bullet->AddComponent<Script_Bullet>();
		break;
	case BulletType::Missile:
		bulletScript = bullet->AddComponent<Script_Missile>();
		break;
	default:
		assert(0);
		break;
	}
	bulletScript->Init();
	bulletScript->SetDamage(damage);
	bulletScript->SetSpeed(speed);

	for (int bulletType = 0; bulletType < BulletPSTypeCount; ++bulletType)
		bulletScript->SetParticleSystems(static_cast<BulletPSType>(bulletType), mPSNames[bulletType]);
}


void Script_BulletWeapon::CreateBulletPool()
{
	mBulletPool = Scene::I->CreateObjectPool("bullet", mBulletCntPerMag * mBulletCntPerShot, std::bind(&Script_BulletWeapon::BulletInitFunc, this, std::placeholders::_1));
}

#pragma endregion