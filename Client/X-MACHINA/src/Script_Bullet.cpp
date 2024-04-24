#include "stdafx.h"
#include "Script_Bullet.h"

#include "Script_Enemy.h"
#include "Script_Weapon.h"

#include "Object.h"
#include "Timer.h"
#include "Scene.h"

#include "Component/Rigidbody.h"
#include "Component/ParticleSystem.h"

void Script_Bullet::Awake()
{
	base::Awake();

	mGameObject = mObject->GetObj<GameObject>();

	const auto& rb = mObject->GetComponent<Rigidbody>();
	rb->SetFriction(0.001f);
	rb->SetDrag(0.001f);

	Reset();
}

void Script_Bullet::SetParticleSystems(BulletPSType type, const std::vector<std::string>& psNames)
{
	for (auto& name : psNames) {
		mPSs[static_cast<UINT8>(type)].emplace_back(mGameObject->AddComponent<ParticleSystem>()->Load(name))->Awake();
	}

	mIsSetPSs = true;
}

void Script_Bullet::Update()
{
	mCurrLifeTime += DeltaTime();

	if (mCurrLifeTime >= mMaxLifeTime) {
		Reset();
		StopPSs(BulletPSType::Contrail);
		mGameObject->OnDestroy();
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

void Script_Bullet::Fire(const Vec3& pos, const Vec3& dir, const Vec3& up)
{
	mObject->SetPosition(pos);

	const auto& rigid = mObject->GetComponent<Rigidbody>();
	rigid->Stop();
	rigid->AddForce(dir, mSpeed, ForceMode::Impulse);

	SetDamage(mDamage);
}

void Script_Bullet::Fire(const Transform& transform)
{
	ResetPSs(BulletPSType::Explosion);
	PlayPSs(BulletPSType::Contrail);

	mObject->SetLocalRotation(transform.GetRotation());
	Fire(transform.GetPosition(), transform.GetLook(), transform.GetUp());
}

void Script_Bullet::Explode()
{
	if (!mGameObject->IsActive()) {
		return;
	}

	Reset();
	StopPSs(BulletPSType::Contrail);

	mGameObject->OnDestroy();
}

void Script_Bullet::PlayPSs(BulletPSType type)
{
	for (const auto& ps : mPSs[static_cast<UINT8>(type)]) 
		ps->Play();
}

void Script_Bullet::StopPSs(BulletPSType type)
{
	for (const auto& ps : mPSs[static_cast<UINT8>(type)])
		ps->Stop();
}

void Script_Bullet::ResetPSs(BulletPSType type)
{
	for (const auto& ps : mPSs[static_cast<UINT8>(type)])
		ps->Reset();
}

void Script_Bullet::Reset()
{
	mCurrLifeTime = 0.f;
}

bool Script_Bullet::IntersectTerrain()
{
	const Vec3 pos = mObject->GetPosition();
	const float terrainHeight = Scene::I->GetTerrainHeight(pos.x, pos.z);

	if (pos.y <= terrainHeight) {
		return true;
	}

	return false;
}