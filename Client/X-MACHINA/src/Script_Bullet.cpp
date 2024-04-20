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
	mParticleSystem = mGameObject->AddComponent<ParticleSystem>()->Load("Bulletlight2");

	mRigid = mObject->GetComponent<Rigidbody>();
	mRigid->SetFriction(0.001f);
	mRigid->SetDrag(0.001f);

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
		Explode();
		break;

	case ObjectTag::Enemy:
	{
		auto& enemy = other.GetComponent<Script_Enemy>();
		enemy->Hit(GetDamage());
		mParticleSystem->Play();
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

	mRigid->Stop();
	mRigid->AddForce(dir, mSpeed, ForceMode::Impulse);

	SetDamage(mDamage);
}

void Script_Bullet::Fire(const Transform& transform, const Vec2& err)
{
	mObject->SetLocalRotation(transform.GetRotation());
	Vec3 dir = transform.GetLook();
	dir = Vector3::Rotate(dir, err.y, err.x, 0.f);
	mObject->Rotate(err.y, err.x, 0.f);
	Fire(transform.GetPosition(), dir, transform.GetUp());
}

void Script_Bullet::Explode()
{
	if (!mGameObject->IsActive()) {
		return;
	}

	Reset();

	mRigid->Stop();
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