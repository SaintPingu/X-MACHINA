#include "stdafx.h"
#include "Script_Bullet.h"

#include "Object.h"
#include "Timer.h"
#include "Scene.h"
#include "Component/Rigidbody.h"


void Script_Bullet::Awake()
{
	mGameObject = mObject->GetObj<GameObject>();

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
		Explode();
		break;
	}
}

void Script_Bullet::Fire(const Vec3& pos, const Vec3& dir, const Vec3& up, float speed)
{
	mObject->SetPosition(pos);

	const auto& rigid = mObject->GetComponent<Rigidbody>();
	rigid->Stop();
	rigid->AddForce(dir, speed, ForceMode::Impulse);
}

void Script_Bullet::Fire(const Transform& transform, float speed)
{
	mObject->SetRotation(transform.GetRotation());
	Fire(transform.GetPosition(), transform.GetLook(), transform.GetUp(), speed);
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
	const float terrainHeight = scene->GetTerrainHeight(pos.x, pos.z);

	if (pos.y <= terrainHeight) {
		return true;
	}

	return false;
}