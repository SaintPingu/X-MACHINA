#include "stdafx.h"
#include "Script_SpiderMine.h"

#include "Script_LiveObject.h"
#include "Script_MainCamera.h"

#include "Component/Collider.h"
#include "Component/Camera.h"

#include "Object.h"
#include "BattleScene.h"
#include "Timer.h"


void Script_SpiderMine::Awake()
{
	base::Awake();
	mMass = 0.6f;
	mDrag = 3.f;
	mRotationSpeed = 90.f;
	mRotationXSpeed = 10.f;
	mRotationX = -45.f;
	mPlantY = 0.5f;

	SetExplosionTag(ObjectTag::Enemy);
}

void Script_SpiderMine::Update()
{
	if (mIsPlanted) {
		mObject->Rotate(0, mRotationSpeed * DeltaTime(), 0);
		return;
	}

	base::Update();

	Move();
}

void Script_SpiderMine::OnCollisionEnter(Object& other)
{
	if (!mIsPlanted) {
		switch (other.GetTag()) {
		case ObjectTag::Bound:
		case ObjectTag::Building:
		case ObjectTag::DissolveBuilding:
			mSpeed *= -1;
			break;
		}
		return;
	}

	base::OnCollisionEnter(other);
}

void Script_SpiderMine::Fire(const Vec3& pos, const Vec3& dir)
{
	mObject->SetPosition(pos);
	mObject->SetLook(dir);
	mObject->Rotate(mRotationX, 0, 0);

	SetDamage(GetDamage());
}

void Script_SpiderMine::StartFire()
{
	base::StartFire();
}

void Script_SpiderMine::Move()
{
	mObject->MoveUp(-Math::kGravity * mMass * DeltaTime());
	if (mObject->GetPosition().y <= mPlantY) {
		Plant();
		return;
	}

	mSpeed -= Math::Sign(mSpeed) * DeltaTime() * mDrag;

	if (mRotationX < 0) {
		float rotationAmount = mRotationXSpeed * DeltaTime();
		mRotationX += rotationAmount;
		if (mRotationX >= 0) {
			rotationAmount -= mRotationX;
			mRotationX = 0;
		}
		mObject->Rotate(rotationAmount, 0, 0);
	}
}

void Script_SpiderMine::Plant()
{
	auto collisionObjects = mObject->GetCollisionObjects(); // copy
	for(const auto& object : collisionObjects) {
		if (object->GetTag() == ObjectTag::Enemy) {
			Explode();
		}
	}

	mIsPlanted = true;
	mObject->Rotate(-mRotationX);
	mObject->SetPositionY(mPlantY);

	if (IsPlayerBullet()) {
		// send here
	}
}
