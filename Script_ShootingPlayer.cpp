#include "stdafx.h"
#include "Script_Player.h"
#include "InputMgr.h"

#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "Timer.h"
#include "Object.h"
#include "Rigidbody.h"
#include "ObjectPool.h"

#include "Script_Bullet.h"



void Script_ShootingPlayer::Update()
{
	base::Update();

	if (mBulletPool) {
		mBulletPool->DoActiveObjects([this](rsptr<InstObject> object) {
			object->Update();
			});

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


void Script_ShootingPlayer::RenderBullets() const
{
	if (mBulletPool) {
		mBulletPool->Render();
	}
}


void Script_ShootingPlayer::CreateBullets(rsptr<const MasterModel> bulletModel)
{
	constexpr int kBulletPoolSize = 10;

	mBulletPool = std::make_shared<ObjectPool>(bulletModel, kBulletPoolSize, sizeof(SB_ColorInst));
	mBulletPool->CreateObjects<InstBulletObject>([this](rsptr<InstObject> object) { InitBullet(object); });
}

void Script_ShootingPlayer::InitBullet(rsptr<InstObject> object)
{
	constexpr float kBulletLifeTime = 3.f;

	object->SetTag(ObjectTag::Bullet);
	object->AddComponent<Rigidbody>();
	const auto& script_bullet = object->AddComponent<Script_Bullet>();
	script_bullet->SetOwner(mObject);
	script_bullet->SetLifeTime(kBulletLifeTime);
}

void Script_ShootingPlayer::FireBullet(const Vec3& pos, const Vec3& dir, const Vec3& up)
{
	const auto& bullet = mBulletPool->Get();
	if (bullet) {
		bullet->GetComponent<Script_Bullet>()->Fire(pos, dir, up, mBulletSpeed);
	}
}