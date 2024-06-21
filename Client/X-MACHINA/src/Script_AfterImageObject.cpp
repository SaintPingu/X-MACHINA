#include "stdafx.h"
#include "Script_AfterImageObject.h"

#include "Scene.h"
#include "Object.h"
#include "Timer.h"
#include "Animator.h"

void Script_AfterImageObject::SetAfterImage(UINT createCnt, float lifeTime)
{
	mRateOverCreateCnt = createCnt;
	mAfterImageLifeTime = lifeTime;
}

void Script_AfterImageObject::OnEnable()
{
	base::OnEnable();

	mAccTime = 0.f;
	mCurrObjectIndex = 0;

	for (auto& object : mAfterImageObjects) {
		object->mObjectCB.HitRimColor = Vec3{ 2.f, 1.f, 2.f };
		object->mObjectCB.HitRimFactor = 1.f;
	}
}

void Script_AfterImageObject::Awake()
{
	base::Awake();

	mAfterImageObjects.resize(mkMaxCreateCnt);
	for (auto& object : mAfterImageObjects) {
		sptr<GameObject> gameObject = std::dynamic_pointer_cast<GameObject>(Scene::I->Instantiate("EliteTrooper", ObjectTag::Unspecified));
		if (gameObject) {
			object = gameObject;
		}

		object->SetUseShadow(false);
	}
}

void Script_AfterImageObject::Update()
{
	mAccTime += DeltaTime();
	
	const float createInterval = 1.f / static_cast<float>(mRateOverCreateCnt);
	if (mAccTime >= createInterval) {
		PushObject();
		mAccTime = 0.f;
	}

	for (int i = 0; i < mCurrObjectIndex; ++i) {
		const float reductionTime = DeltaTime() / mAfterImageLifeTime;
		mAfterImageObjects[i]->mObjectCB.HitRimFactor -= reductionTime;
	}

	if (mAfterImageObjects.front()->mObjectCB.HitRimFactor <= 0.f) {
		PopObject();
	}
}

void Script_AfterImageObject::PushObject()
{
	if (mCurrObjectIndex >= mkMaxCreateCnt) {
		return;
	}

	mAfterImageObjects[mCurrObjectIndex]->mObjectCB.HitRimFactor = 1.f;
	mAfterImageObjects[mCurrObjectIndex]->SetPosition(mObject->GetPosition());
	mAfterImageObjects[mCurrObjectIndex]->SetLocalRotation(mObject->GetRotation());
	mAfterImageObjects[mCurrObjectIndex]->GetAnimator()->CloneBoneFrames(mObject);
	mCurrObjectIndex++;
}

void Script_AfterImageObject::PopObject()
{
	auto& front = mAfterImageObjects.front();
	mAfterImageObjects.pop_front();
	mAfterImageObjects.push_back(front);
	mCurrObjectIndex--;
}


