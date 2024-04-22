#include "stdafx.h"
#include "Script_Bullet.h"

#include "Script_Enemy.h"

#include "Object.h"
#include "Timer.h"
#include "Scene.h"

#include "Component/Rigidbody.h"
#include "Component/ParticleSystem.h"

void Script_Bullet::Awake()
{
	base::Awake();

	mGameObject = mObject->GetObj<GameObject>();
	mParticleSystems.reserve(10);
	mParticleSystems.emplace_back(mCollisionSmokePS = mGameObject->AddComponent<ParticleSystem>()->Load("WFX_Smoke_BigQuick"));
	mParticleSystems.emplace_back(mGameObject->AddComponent<ParticleSystem>()->Load("WFX_Smoke"));
	mParticleSystems.emplace_back(mGameObject->AddComponent<ParticleSystem>()->Load("WFX_Glow"));
	mParticleSystems.emplace_back(mGameObject->AddComponent<ParticleSystem>()->Load("WFX_Explosion"));
	mParticleSystems.emplace_back(mGameObject->AddComponent<ParticleSystem>()->Load("WFX_Explosion_Small"));
	mParticleSystems.emplace_back(mGameObject->AddComponent<ParticleSystem>()->Load("WFX_Dot_Sparkles"));
	mParticleSystems.emplace_back(mGameObject->AddComponent<ParticleSystem>()->Load("WFX_Dot_Sparkles_Big"));
	mParticleSystems.emplace_back(mGameObject->AddComponent<ParticleSystem>()->Load("WFX_Dot_Sparkles_Mult"));
	for (auto& ps : mParticleSystems)
		ps->Awake();

	mBulletSmokePS = mGameObject->AddComponent<ParticleSystem>()->Load("WFX_Bullet");
	mBulletSmokePS->Awake();

	const auto& rb = mObject->GetComponent<Rigidbody>();
	rb->SetFriction(0.001f);
	rb->SetDrag(0.001f);

	Reset();
}

void Script_Bullet::Update()
{
	mCurrLifeTime += DeltaTime();

	if (mCurrLifeTime >= mMaxLifeTime) {
		Reset();
		mBulletSmokePS->Stop();
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
		mCollisionSmokePS->Play();
		Explode();
		break;

	case ObjectTag::Enemy:
	{
		auto& enemy = other.GetComponent<Script_Enemy>();
		enemy->Hit(GetDamage());

		for (const auto& ps : mParticleSystems)
			ps->Play();

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
	for (auto& ps : mParticleSystems)
		ps->Reset();

	mBulletSmokePS->Play();

	mObject->SetLocalRotation(transform.GetRotation());
	Fire(transform.GetPosition(), transform.GetLook(), transform.GetUp());
}

void Script_Bullet::Explode()
{
	if (!mGameObject->IsActive()) {
		return;
	}

	Reset();
	mBulletSmokePS->Stop();

	mGameObject->OnDestroy();
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