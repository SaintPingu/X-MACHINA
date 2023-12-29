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

		m_fCrntFireDelay += DeltaTime();
		if (mIsShooting) {
			if (m_fCrntFireDelay < m_fFireDelay) {
				return;
			}

			m_fCrntFireDelay = 0.0f;
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
		mBulletShader->Render();
	}
}



const std::list<sptr<GameObject>>* Script_ShootingPlayer::GetBullets() const
{
	if (mBulletShader) {
		return mBulletShader->GetBullets();
	}
	else {
		return nullptr;
	}
}


void Script_ShootingPlayer::CreateBullets(rsptr<const MasterModel> bulletModel)
{
	mBulletShader = std::make_shared<BulletShader>();
	mBulletShader->CreateShader();
	mBulletShader->BuildObjects(100, bulletModel, mObject);
	mBulletShader->SetLifeTime(mbulletLifeTime);

	constexpr Vec3 color{ 1.f, 1.f, 0.f };
	mBulletShader->SetColor(Vec3(color.x, color.y, color.z));
}