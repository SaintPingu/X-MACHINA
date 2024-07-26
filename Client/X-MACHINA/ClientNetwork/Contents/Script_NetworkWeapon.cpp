#include "stdafx.h"
#include "Script_NetworkBulletWeapon.h"

#include "Script_Bullet.h"
#include "Script_Missile.h"
#include "Script_Player.h"

#include "Component/Rigidbody.h"
#include "Component/ParticleSystem.h"

#include "BattleScene.h"
#include "Object.h"
#include "ObjectPool.h"
#include "Timer.h"

#include "FBsPacketFactory.h"
#include "ClientNetworkManager.h"

#pragma region Script_NetworkBulletWeapon
void Script_NetworkBulletWeapon::FireBullet()
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

void Script_NetworkBulletWeapon::InitBullet(rsptr<InstObject> bullet, float damage, float speed, BulletType bulletType) const
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


void Script_NetworkBulletWeapon::CreateBulletPool()
{
	mBulletPool = BattleScene::I->CreateObjectPool("bullet", mBulletCntPerMag * mBulletCntPerShot, std::bind(&Script_NetworkBulletWeapon::BulletInitFunc, this, std::placeholders::_1));
}

#pragma endregion