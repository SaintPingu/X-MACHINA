#include "stdafx.h"
#include "Script_Player.h"
#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "Timer.h"
#include "InputMgr.h"




void Script_ShootingPlayer::Start()
{
	base::Start();

	if (mBulletShader) {
		mBulletShader->Start();
	}
}


void Script_ShootingPlayer::Update()
{
	base::Update();

	if (mBulletShader) {
		mBulletShader->Update();

		mCurrFireDelay += DeltaTime();
		if (mIsShooting) {
			if (mCurrFireDelay < mMaxFireDelay) {
				return;
			}

			mCurrFireDelay = 0.f;
			FireBullet();
		}
	}
}

void Script_ShootingPlayer::ProcessInput()
{
	if (KEY_TAP(VK_LCONTROL)) {
		StartFire();
	}
	else if (KEY_AWAY(VK_LCONTROL)) {
		StopFire();
	}
}


void Script_ShootingPlayer::SetDamage(float damage)
{
	if (mBulletShader) {
		mBulletShader->SetDamage(damage);
	}
}


void Script_ShootingPlayer::RenderBullets() const
{
	if (mBulletShader) {
		mBulletShader->Set();
		mBulletShader->Render();
	}
}


void Script_ShootingPlayer::CreateBullets(rsptr<const MasterModel> bulletModel)
{
	static constexpr float kBulletLifeTime = 10.f;

	mBulletShader = std::make_shared<BulletShader>();
	mBulletShader->Create();
	mBulletShader->BuildObjects(bulletModel, mObject);
	mBulletShader->SetLifeTime(kBulletLifeTime);

	constexpr Vec3 kColor{ 1.f, 1.f, 0.f };
	mBulletShader->SetColor(Vec3(kColor.x, kColor.y, kColor.z));
}