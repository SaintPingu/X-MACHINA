#include "stdafx.h"
#include "Airstrike.h"

#include "Script_Missile.h"

#include "Component/ParticleSystem.h"

#include "BattleScene.h"
#include "Object.h"
#include "ObjectPool.h"
#include "SoundMgr.h"
#include "Timer.h"

#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"


void Airstrike::Init()
{
	mMaxMissileCnt = 24;
	mExpDamage = 40.f;
	mMissileSpeed = 20.f;
	mMaxStrikeDelay = 1.5f;
	mMaxFireDelay = 0.35f;
	mCntPerFire = 3;
	mErrX = Vec2(-3, 3);
	mErrY = Vec2(-3, 3);
	SetFireSound("Airstrike Fire");

	SetParticleSystemNames();
	CreateMissilePool();

	On();
}

void Airstrike::Update()
{
	mCurStrikeDelay -= DeltaTime();
	if (mCurStrikeDelay < 0) {
		mCurStrikeDelay = 0;
	}

	if (!mIsFired) {
		return;
	}

	mCurFireDelay -= DeltaTime();
	if (mCurFireDelay > 0) {
		return;
	}
	mCurFireDelay = mMaxFireDelay;

	Fire();

	--mRemainMissileCnt;
	--mCurFireCnt;
	if (mCurFireCnt <= 0 || mRemainMissileCnt <= 0) {
		mIsFired = false;
	}
}

void Airstrike::On()
{
	mIsFired = false;
	mRemainMissileCnt = mMaxMissileCnt;
	mCurStrikeDelay = 0;
}

void Airstrike::StartFire(const Vec3& pos, const Vec3& dir)
{
	if (mIsFired || mRemainMissileCnt <= 0) {
		return;
	}

	if (mCurStrikeDelay > 0) {
		return;
	}
	mCurStrikeDelay = mMaxStrikeDelay;

	mIsFired = true;
	mFirePos = pos;
	mFireDir = dir;
	mCurFireCnt = mCntPerFire;
	mCurFireDelay = 0;
}

void Airstrike::Fire(const Vec3& pos, const Vec3& dir)
{
	const auto& missile = mMissilePool->Get(true);
	if (missile) {
		auto& missileScript = missile->GetComponent<Script_Missile>();
		missileScript->Fire(pos, dir);
	}
}

void Airstrike::SetPlayerBullet(const Object* player)
{
	mMissilePool->DoAllObjects([&](rsptr<InstObject> bullet) {
		bullet->GetComponent<Script_Bullet>()->SetPlayerBullet(true, player);
		});
}


void Airstrike::CreateMissilePool()
{
	mMissilePool = BattleScene::I->CreateObjectPool("SM_Missile", mMaxMissileCnt, std::bind(&Airstrike::MissileInitFunc, this, std::placeholders::_1));
}


void Airstrike::Fire()
{
	const auto& missile = mMissilePool->Get(true);
	if (missile) {
		auto& missileScript = missile->GetComponent<Script_Missile>();

		const Vec2 err = Vec2(Math::RandFloat(mErrX.x, mErrX.y), Math::RandFloat(mErrY.x, mErrY.y));
		missileScript->Fire(mFirePos, mFireDir, err);
		if (mFireSound != "") {
			SoundMgr::I->Play("Gun", mFireSound);
		}

#ifdef SERVER_COMMUNICATION
		auto cpkt = FBS_FACTORY->CPkt_Bullet_OnShoot(missile->GetPosition(), missile->GetLook());
		CLIENT_NETWORK->Send(cpkt);
#endif
	}

}


void Airstrike::MissileInitFunc(rsptr<InstObject> missile) const
{
	sptr<Script_Missile> missileScript = missile->AddComponent<Script_Missile>();

	missileScript->Init();
	missileScript->SetDamage(0);
	missileScript->SetSpeed(mMissileSpeed);
	missileScript->SetExplosionDamage(mExpDamage);
	missileScript->SetImpactSound("Burnout Impact");
	missileScript->SetEndDistance(100.f);
	missileScript->NoCollisionEnemy();

	for (int bulletType = 0; bulletType < BulletPSTypeCount; ++bulletType)
		missileScript->SetParticleSystems(static_cast<BulletPSType>(bulletType), mPSNames[bulletType]);
}

void Airstrike::SetParticleSystemNames()
{
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Airstrike_Explosion_Smoke_Spread");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Airstrike_Explosion_Smoke");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Airstrike_Explosion_Add");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Airstrike_Dot_Sparkles");

	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Airstrike_Explosion_Smoke_Spread");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Airstrike_Explosion_Smoke");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Airstrike_Explosion_Add");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Airstrike_Dot_Sparkles");
}