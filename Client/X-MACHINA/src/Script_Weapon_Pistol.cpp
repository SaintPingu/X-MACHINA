#include "stdafx.h"
#include "Script_Weapon_Pistol.h"

#include "Script_Bullet.h"
#include "Component/Rigidbody.h"

#include "Object.h"
#include "ObjectPool.h"
#include "Scene.h"


static void BulletInitFunc(rsptr<InstObject> bullet)
{
	bullet->SetTag(ObjectTag::Bullet);
	bullet->AddComponent<Script_Bullet>();
	bullet->AddComponent<Rigidbody>();
}


void Script_Weapon_Pistol::Start()
{
	// default value //
	mBulletDamage  = 8.f;
	mBulletSpeed   = 30.f;
	mMaxFireDelay  = 0.1f;
	mMaxReloadTime = 1.7f;
	mMaxDistance   = 16.f;

	mMaxMag = 15;
}

void Script_Weapon_Pistol::CreateBulletPool()
{
	mBulletPool = scene->CreateObjectPool("bullet", 200, BulletInitFunc);
}

void Script_Weapon_Pistol::FireBullet()
{
	auto& bullet = mBulletPool->Get(true);
	if (bullet) {
		auto& bulletScript = bullet->GetComponent<Script_Bullet>();
		bulletScript->Fire(*mMuzzle, mBulletSpeed, mBulletDamage);
	}
}