#include "stdafx.h"
#include "Script_Weapon_Pistol.h"

#include "Script_Bullet.h"
#include "Component/Rigidbody.h"

#include "Object.h"
#include "ObjectPool.h"
#include "Scene.h"


void Script_Weapon_Pistol::Awake()
{
	base::Awake();

	InitValues();
	SetFiringMode(FiringMode::SemiAuto);
}

void Script_Weapon_Pistol::CreateBulletPool()
{
	mBulletPool = scene->CreateObjectPool("bullet", 200, std::bind(&Script_Weapon_Pistol::BulletInitFunc, this, std::placeholders::_1));
}

void Script_Weapon_Pistol::FireBullet()
{
	auto& bullet = mBulletPool->Get(true);
	if (bullet) {
		auto& bulletScript = bullet->GetComponent<Script_Bullet>();
		bulletScript->Fire(*mMuzzle);
	}
}

void Script_Weapon_Pistol::InitValues()
{
	mMaxFireDelay  = CalcFireDelay(kRPM);
	mMaxReloadTime = 1.7f;
	mMaxDistance   = 16.f;
	mMaxMag        = 15;
}

void Script_Weapon_Pistol::BulletInitFunc(rsptr<InstObject> bullet) const
{
	bullet->SetTag(ObjectTag::Bullet);

	auto& bulletScript = bullet->AddComponent<Script_Bullet>();
	bulletScript->SetDamage(kBulletDamage);
	bulletScript->SetSpeed(kBulletSpeed);

	bullet->AddComponent<Rigidbody>();
}