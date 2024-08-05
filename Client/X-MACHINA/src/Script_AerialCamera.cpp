#include "stdafx.h"
#include "Script_AerialCamera.h"

#include "Script_MainCamera.h"

#include "Component/Camera.h"

#include "GameFramework.h"
#include "Object.h"
#include "Timer.h"

namespace {
	Vec3 kCameraOffset = Vec3(0.f, 30.f, -7.f);
}


void Script_AerialCamera::Awake()
{
	mStartDist = -10.f;
	mEndDist = 10.f;
	mMovementSpeed = 0.6f;
}

void Script_AerialCamera::Start()
{
	base::Start();

	mMainCamera = mObject->GetComponent<Script_MainCamera>();
}

void Script_AerialCamera::Update()
{
	base::Update();

	Move();
}

void Script_AerialCamera::OnEnable()
{
	base::OnEnable();
	if (!mMainCamera.expired()) {
		mMainCamera.lock()->SetActive(false);
	}

	Init();
}

void Script_AerialCamera::OnDisable()
{
	base::OnDisable();
	if (!mMainCamera.expired()) {
		mMainCamera.lock()->SetActive(true);
	}
}


void Script_AerialCamera::Init()
{
	// values //
	mCurDist = mStartDist;

	// camera //
	Vec3 playerPos = GameFramework::I->GetPlayer()->GetPosition();

	constexpr float kMaxPlaneDistance = 500.f;

	// 플레이어를 바라보는 시점에서 시작
	SetCameraOffset(kCameraOffset);
	mObject->SetPosition(playerPos + mOffset);
	MAIN_CAMERA->LookAt(playerPos, Vector3::Up);

	mObject->MoveStrafe(mStartDist);

	MAIN_CAMERA->SetProjMtx(0.01f, kMaxPlaneDistance, 60.f);
}


void Script_AerialCamera::SetCameraOffset(const Vec3& offset)
{
	mOffset = offset;
	MAIN_CAMERA->SetOffset(mOffset);
}

void Script_AerialCamera::Move()
{
	if (mCurDist >= mEndDist) {
		return;
	}

	float moveAmount = mMovementSpeed * DeltaTime();
	mCurDist += moveAmount;
	if (mCurDist >= mEndDist) {
		moveAmount -= mCurDist - mEndDist;
	}

	mObject->MoveStrafe(moveAmount);
}
