#include "stdafx.h"
#include "Script_Bullet.h"

#include "Script_Enemy.h"
#include "Script_Weapon.h"

#include "Object.h"
#include "Timer.h"
#include "BattleScene.h"

#include "Component/Rigidbody.h"
#include "Component/ParticleSystem.h"

void Script_Bullet::SetParticleSystems(BulletPSType type, const std::vector<std::string>& psNames)
{
	for (auto& name : psNames) {
		mPSNames[static_cast<UINT8>(type)].emplace_back(name);
	}
}

void Script_Bullet::Update()
{
	mCurrLifeTime += DeltaTime();

	if (mCurrLifeTime >= mMaxLifeTime) {
		Reset();
		StopPSs(BulletPSType::Contrail);
		mGameObject->Return();
	}
	else if ((mObject->GetPosition().y < 0.f) || IntersectTerrain()) {
		Explode();
	}

}

void Script_Bullet::OnCollisionStay(Object& other)
{
	if (mCurrLifeTime <= FLT_EPSILON) {
		return;
	}
	if (IsOwner(&other)) {
		return;
	}

	switch (other.GetTag()) {
	case ObjectTag::Building:
	case ObjectTag::DissolveBuilding:
		PlayPSs(BulletPSType::Building);
		Explode();
		break;

	case ObjectTag::Enemy:
	{
		auto& enemy = other.GetComponent<Script_Enemy>();
		enemy->Hit(GetDamage());
		PlayPSs(BulletPSType::Explosion);
		Explode();
	}

	break;
	default:
		break;
	}
}


void Script_Bullet::Init()
{
	mGameObject = mObject->GetObj<InstObject>();
	mObject->SetTag(ObjectTag::Bullet);

	mRigid = mObject->GetComponent<Rigidbody>();
	mRigid->SetFriction(0.001f);
	mRigid->SetDrag(0.001f);

	Reset();
}

void Script_Bullet::Fire(const Vec3& pos, const Vec3& dir, const Vec3& up)
{
	mObject->SetPosition(pos);

	mRigid->Stop();
	mRigid->AddForce(dir, mSpeed, ForceMode::Impulse);

	SetDamage(mDamage);
	StartFire();
}

void Script_Bullet::Fire(const Transform& transform, const Vec2& err)
{
	PlayPSs(BulletPSType::Contrail);

	mObject->SetLocalRotation(transform.GetRotation());
	Vec3 dir = transform.GetLook();
	dir = Vector3::Rotate(dir, err.y, err.x, 0.f);
	mObject->Rotate(err.y, err.x, 0.f);
	Fire(transform.GetPosition(), dir, transform.GetUp());
}

void Script_Bullet::Explode()
{
	Reset();
	StopPSs(BulletPSType::Contrail);

	mRigid->Stop();
	mGameObject->Return();
}

void Script_Bullet::PlayPSs(BulletPSType type)
{
	for (const auto& psName : mPSNames[static_cast<UINT8>(type)]) {
		auto* ps = ParticleManager::I->Play(psName, mObject);

		// 루핑이 설정되어 있다면 직접 해제해야 한다.
		if (ps && ps->GetPSCD()->Looping) {
			mPSs[static_cast<UINT8>(type)].push(ps);
		}
	}
}

void Script_Bullet::StopPSs(BulletPSType type)
{
	while (!mPSs[static_cast<UINT8>(type)].empty()) {
		mPSs[static_cast<UINT8>(type)].front()->Stop();
		mPSs[static_cast<UINT8>(type)].pop();
	}
}

void Script_Bullet::Reset()
{
	mCurrLifeTime = 0.f;
}

bool Script_Bullet::IntersectTerrain()
{
	const Vec3 pos = mObject->GetPosition();
	const float terrainHeight = BattleScene::I->GetTerrainHeight(pos.x, pos.z);

	if (pos.y <= terrainHeight) {
		return true;
	}

	return false;
}