#include "stdafx.h"
#include "Script_Weapon.h"

#include "Script_Bullet.h"
#include "Script_Missile.h"
#include "Script_Player.h"
#include "Script_SpiderMine.h"

#include "Component/Rigidbody.h"
#include "Component/ParticleSystem.h"

#include "GameFramework.h"

#include "BattleScene.h"
#include "Object.h"
#include "ObjectPool.h"
#include "Timer.h"

#include "SoundMgr.h"


#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"

#pragma region Script_Weapon
void Script_Weapon::Awake()
{
	base::Awake();

	mMuzzle = mObject->FindFrame("FirePos");

	SetParticleSystemNames();

	InitValues();
	mMaxMag = 100; // TODO : for demo
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

void Script_Weapon::OnEnable()
{
	base::OnEnable();

	if (IsPlayerWeapon()) {

	}
}

void Script_Weapon::OnDisable()
{
	base::OnDisable();
}

void Script_Weapon::FireBullet()
{
	if (!mOwner) {
		return;
	}

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

void Script_Weapon::SetOwner(Script_GroundPlayer* owner)
{
	mOwner = owner;
	if (mOwner->GetObj() == GameFramework::I->GetPlayer()) {
		mIsPlayerWeapon = true;
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
	//--mCurMag;
	mCurBulletCnt = mBulletCntPerMag;
	mCurReloadTime = 0.f;
	mCurFireDelay = mMaxFireDelay;
	mIsReload = false;
}

bool Script_Weapon::InitReload()
{
	mCurBulletCnt = 0;
	mCurReloadTime = 0.f;
	mIsReload = true;

	return true;
}

void Script_Weapon::StartReload()
{
	if (IsReloading()) {
		return;
	}

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
std::string Script_Weapon::GetWeaponModelName(WeaponName weaponName)
{
	static const std::unordered_map<WeaponName, std::string> kWeaponMaps{
		{WeaponName::H_Lock, "SM_SciFiLaserGun" },
		{WeaponName::SkyLine, "SM_SciFiAssaultRifle_01" },
		{WeaponName::DBMS, "SM_SciFiShotgun" },
		{WeaponName::Burnout, "SM_SciFiMissileLauncher" },
		{WeaponName::PipeLine, "Sniper" },
		{WeaponName::MineLauncher, "SM_SciFiLightingGun" },
	};

	assert(kWeaponMaps.count(weaponName));

	return kWeaponMaps.at(weaponName);
}
int Script_Weapon::GetWeaponIdx(WeaponType weaponType)
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
	case WeaponType::MineLauncher:
		return 2;
		break;
	default:
		break;
	}

	assert(0);
	return 0;
}
#pragma endregion

















#pragma region Script_BulletWeapon
void Script_BulletWeapon::FireBullet()
{
	base::FireBullet();

	const auto& bullet = mBulletPool->Get(true);
	if (bullet) {
		const auto& bulletScript = bullet->GetComponent<Script_Bullet>();

		Vec2 err = Vec2(Math::RandFloat(mErrX.x, mErrX.y), Math::RandFloat(mErrY.x, mErrY.y));
		const float bulletSpeedErr = Math::RandFloat(0, mSpeerErr);

		bulletScript->SetSpeed(GetBulletSpeed() - bulletSpeedErr);

		bulletScript->Fire(*mMuzzle, err);
		if (mFireSound != "") {
			SoundMgr::I->Play("Gun", mFireSound);
		}
	}
}

void Script_BulletWeapon::StartReload()
{
	if (IsReloading()) {
		return;
	}
	base::StartReload();

	if (mReloadSound != "") {
		SoundMgr::I->Play("Reload", mReloadSound);
	}
}

void Script_BulletWeapon::StopReload()
{
	base::StopReload();

	if (mReloadSound != "") {
		SoundMgr::I->Stop("Reload");
	}
}

void Script_BulletWeapon::EndReload()
{
	base::EndReload();

	if (mEndReloadSound != "") {
		SoundMgr::I->PlayNoChannel("Reload", mEndReloadSound);
	}
}

void Script_BulletWeapon::InitBullet(rsptr<InstObject> bullet, float damage, float speed, BulletType bulletType) const
{
	sptr<Script_Bullet> bulletScript{};
	switch (bulletType) {
	case BulletType::Bullet:
		bulletScript = bullet->AddComponent<Script_Bullet>();
		break;
	case BulletType::Missile:
		bulletScript = bullet->AddComponent<Script_Missile>();
		bullet->SetTag(ObjectTag::Bullet);
		break;
	case BulletType::Mine:
		bulletScript = bullet->AddComponent<Script_SpiderMine>();
		bullet->SetTag(ObjectTag::Bullet);
		break;
	default:
		assert(0);
		break;
	}
	bulletScript->Init();
	if (IsPlayerWeapon()) {
		bulletScript->SetPlayerBullet();
	}
	bulletScript->SetDamage(damage);
	bulletScript->SetSpeed(speed);

	for (int bulletType = 0; bulletType < BulletPSTypeCount; ++bulletType)
		bulletScript->SetParticleSystems(static_cast<BulletPSType>(bulletType), mPSNames[bulletType]);
}


void Script_BulletWeapon::CreateBulletPool()
{
	mBulletPool = BattleScene::I->CreateObjectPool("bullet", mBulletCntPerMag * mBulletCntPerShot, std::bind(&Script_BulletWeapon::BulletInitFunc, this, std::placeholders::_1));
}

#pragma endregion