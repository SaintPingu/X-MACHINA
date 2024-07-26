#include "stdafx.h"
#include "SCript_NetworkWeapon.h"

#include "Script_Bullet.h"
#include "Script_Missile.h"
#include "Script_Player.h"

#include "Component/Rigidbody.h"
#include "Component/ParticleSystem.h"

#include "BattleScene.h"
#include "Object.h"
#include "ObjectPool.h"
#include "Timer.h"





#pragma region Script_NetworkWeapon
void Script_NetworkWeapon::Awake()
{
	base::Awake();

	mMuzzle = mObject->FindFrame("FirePos");

	SetParticleSystemNames();

	InitValues();
	mMaxMag = 100; // TODO : for demo
	CreateBulletPool();
}

void Script_NetworkWeapon::Start()
{
	base::Start();

	mCurMag = mMaxMag;
	mCurBulletCnt = mBulletCntPerMag;
	mCurFireDelay = mMaxFireDelay;
}

void Script_NetworkWeapon::Update()
{
	if (!mOwner || (mOwner->IsInDraw() || mOwner->IsInPutBack() || mIsReload || mCurBulletCnt <= 0)) {
		return;
	}

	if (CanFire()) {
		updateFunc();
	}
	else {
		mCurFireDelay += DeltaTime();
	}
}

void Script_NetworkWeapon::FireBullet()
{
	if (!mOwner) {
		return;
	}

	--mCurBulletCnt;
	ParticleManager::I->Play("WFX_Muzzle_Flash", mMuzzle);
	ParticleManager::I->Play("WFX_Muzzle_Smoke", mMuzzle);

	mOwner->BulletFired();
}

void Script_NetworkWeapon::SetFiringMode(FiringMode firingMode)
{
	switch (firingMode) {
	case FiringMode::Auto:
		updateFunc = std::bind(&Script_NetworkWeapon::Update_Auto, this);
		break;
	case FiringMode::SemiAuto:
		updateFunc = std::bind(&Script_NetworkWeapon::Update_SemiAuto, this);
		break;
	case FiringMode::BoltAction:
		updateFunc = std::bind(&Script_NetworkWeapon::Update_BoltAction, this);
		break;
	default:
		assert(0);
		break;
	}
}

bool Script_NetworkWeapon::CheckReload()
{
	if (mCurMag <= 0) {	// 모든 탄창 소진
		return false;
	}

	StartReload();

	return true;
}

void Script_NetworkWeapon::EndReload()
{
	--mCurMag;
	mCurBulletCnt = mBulletCntPerMag;
	mCurReloadTime = 0.f;
	mCurFireDelay = mMaxFireDelay;
	mIsReload = false;
}

bool Script_NetworkWeapon::InitReload()
{
	mCurBulletCnt = mCurBulletCnt > 0 ? 1 : 0;  // 약실 -> 1발 장전 상태
	mCurReloadTime = 0.f;
	mIsReload = true;

	return true;
}

void Script_NetworkWeapon::StartReload()
{
	InitReload();

	if (mOwner) {
		mOwner->StartReload();
	}
}

void Script_NetworkWeapon::Update_Auto()
{
	if (mIsShooting) {
		Fire();
	}
}

void Script_NetworkWeapon::Update_SemiAuto()
{
	if (mIsShooting && !mIsBeforeShooting) {
		Fire();
		mIsBeforeShooting = true;
	}
}

void Script_NetworkWeapon::Update_BoltAction()
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


void Script_NetworkWeapon::Fire()
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
std::string Script_NetworkWeapon::GetWeaponModelName(WeaponName weaponName)
{
	static const std::unordered_map<WeaponName, std::string> kWeaponMaps{
		{WeaponName::H_Lock, "SM_SciFiLaserGun" },
		{WeaponName::SkyLine, "SM_SciFiAssaultRifle_01" },
		{WeaponName::DBMS, "SM_SciFiShotgun" },
		{WeaponName::Burnout, "SM_SciFiMissileLauncher" },
		{WeaponName::PipeLine, "Sniper" },
	};

	assert(kWeaponMaps.count(weaponName));

	return kWeaponMaps.at(weaponName);
}
int Script_NetworkWeapon::GetWeaponIdx(WeaponType weaponType)
{
	switch (weaponType) {
	case WeaponType::HandedGun:
		return 0;
		break;
	case WeaponType::ShotGun:
	case WeaponType::AssaultRifle:
		return 1;
		break;
	case WeaponType::MissileLauncher:
	case WeaponType::Sniper:
		return 2;
		break;
	default:
		break;
	}

	assert(0);
	return 0;
}
#pragma endregion
















