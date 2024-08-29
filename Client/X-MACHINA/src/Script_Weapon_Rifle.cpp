#include "stdafx.h"
#include "Script_Weapon_Rifle.h"
#include "Script_MainCamera.h"
#include "Component/ParticleSystem.h"
#include "Component/Camera.h"

#include "Timer.h"



#pragma region Script_Weapon_Rifle
void Script_Weapon_Rifle::Awake()
{
	base::Awake();

	SetFiringMode(FiringMode::BoltAction);
}

void Script_Weapon_Rifle::Update()
{
	base::Update();

	// ź���� ��ȭ
	if (!IsShooting()) {
		auto RecoverErr = [this](float& err) {
			if (err > 0) {
				err -= mErrRecoverSpeed * DeltaTime();
				if (err < 0) {
					err = 0;
				}
			}
			};

		RecoverErr(mCurErr.x);
		RecoverErr(mCurErr.y);
	}
}

void Script_Weapon_Rifle::FireBullet()
{
	base::FireBullet();

	// ���� ź���� ����
	mCurErr.x += mErrIncAmount;
	mCurErr.y += mErrIncAmount;

	// �ִ� ����
	mCurErr.x = std::clamp(mCurErr.x, -mMaxErrX, mMaxErrX);
	mCurErr.y = std::clamp(mCurErr.y, -mMaxErrY, mMaxErrY);
	
	// �� ����
	mErrX = Vec2(-mCurErr.x, mCurErr.x);
	mErrY = Vec2(-mCurErr.y, mCurErr.y);

	MainCamera::I->GetComponent<Script_MainCamera>()->StartShake(0.1f, 0.0005f);
}
#pragma endregion





#pragma region Script_Weapon_Skyline
void Script_Weapon_Skyline::InitValues()
{
	mMaxFireDelay     = CalcFireDelay(mkRPM);
	mMaxReloadTime    = 2.1f;
	mMaxDistance      = 35.f;
	mBulletCntPerMag  = 35;
	mMaxMag           = 6;

	mMaxErrX = 8.f;
	mMaxErrX = 3.f;

	SetFireSound("Skyline Fire");
	SetReloadSound("Skyline Reload");
}

void Script_Weapon_Skyline::BulletInitFunc(rsptr<InstObject> bullet) const
{
	base::InitBullet(bullet, mkBulletDamage, mkBulletSpeed);
}

void Script_Weapon_Skyline::SetParticleSystemNames()
{
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Smoke_BigQuick");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Smoke");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Dot_Sparkles");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Glow");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Explosion");

	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Smoke_BigQuick");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Smoke");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Dot_Sparkles");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Glow");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Explosion");

	mPSNames[static_cast<UINT8>(BulletPSType::Contrail)].push_back("WFX_Bullet");
}

#pragma endregion

