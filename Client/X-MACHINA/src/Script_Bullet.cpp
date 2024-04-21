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
	mParticleSystems.reserve(7);
	mParticleSystems.emplace_back(mGameObject->AddComponent<ParticleSystem>()->Load("Explosion"));
	mParticleSystems.emplace_back(mGameObject->AddComponent<ParticleSystem>()->Load("Explosion_BigQuick"));
	mParticleSystems.emplace_back(mGameObject->AddComponent<ParticleSystem>()->Load("Explosion_Grow"));
	mParticleSystems.emplace_back(mGameObject->AddComponent<ParticleSystem>()->Load("Explosion_Small"));
	mParticleSystems.emplace_back(mGameObject->AddComponent<ParticleSystem>()->Load("Explosion_Sparkles"));
	mParticleSystems.emplace_back(mGameObject->AddComponent<ParticleSystem>()->Load("Explosion_Sparkles_Big"));
	mParticleSystems.emplace_back(mGameObject->AddComponent<ParticleSystem>()->Load("Explosion_Sparkles_Mult"));
	mSmokeParticleIdx = 1;

	for (auto& ps : mParticleSystems) {
		ps->Awake();
	}

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
		mParticleSystems[mSmokeParticleIdx]->Play();
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
	mObject->SetLocalRotation(transform.GetRotation());
	Fire(transform.GetPosition(), transform.GetLook(), transform.GetUp());
}

void Script_Bullet::Explode()
{
	if (!mGameObject->IsActive()) {
		return;
	}

	Reset();

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