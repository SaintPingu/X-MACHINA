#include "stdafx.h"
#include "Script_MainCamera.h"

#include "Object.h"
#include "Timer.h"
#include "Component/Camera.h"
#include "Scene.h"

#include "X-Engine.h"


void Script_MainCamera::SetCameraOffset(const Vec3& offset)
{
	mMainOffset = offset;
	MAIN_CAMERA->SetOffset(mMainOffset);
}


void Script_MainCamera::Awake()
{
	base::Awake();

	constexpr Vec2 maxOffset = Vec2(3, 3);
	const Vec2 resolution = Vec2(MAIN_CAMERA->GetWidth(), MAIN_CAMERA->GetHeight());

	// corr = 0.002 : 1280 * 1080 -> 2.56, 2.16
	constexpr float corr = 0.002f;
	mMaxOffset = Vec2(resolution.x * corr, resolution.y * corr);

	mMaxOffset.x = mMaxOffset.x > maxOffset.x ? maxOffset.x : mMaxOffset.x;
	mMaxOffset.y = mMaxOffset.y > maxOffset.y ? maxOffset.y : mMaxOffset.y;

	mPlayer = Engine::I->GetPlayer();
}

void Script_MainCamera::Start()
{
	base::Start();

	Init();
}

void Script_MainCamera::Update()
{
	Vec3 offset = mMainOffset + Vec3(mExtraOffset.x, 0.f, mExtraOffset.y);
	mObject->SetPosition(mPlayer->GetPosition() + offset);
}

void Script_MainCamera::Move(Vec2 dir, float maxOffset_t, bool isAlign)
{
	maxOffset_t = std::clamp(maxOffset_t, 0.f, 1.f);
	const Vec2 maxOffset = mMaxOffset * maxOffset_t;

	constexpr float originSpeed = 1.f;

	auto calculateDir = [&](float dir, float extraOffset) -> float {
		if (Math::IsZero(dir)) {
			if (isAlign) {
				return (fabs(extraOffset) < 0.1f) ? 0.f : -Math::Sign(extraOffset);
			}
			else {
				return 0.f;
			}
		}
		else {
			return static_cast<float>(Math::Sign(dir));
		}
		};

	auto calculateSpeed = [&](float dir, float extraOffset, float maxOffset) -> float {
		// 반대 방향 이동시 기본 속도
		if (Math::Sign(extraOffset) != Math::Sign(dir)) {
			return originSpeed;
		}
		// 정방향 이동 시 거리가 멀어질 수록 속도를 줄인다.
		else {
			constexpr float ratioAffect = 2.f;
			float ratio = (fabs(extraOffset) / fabs(maxOffset)) * ratioAffect;
			return originSpeed / (1 + ratio);
		}
		};

	dir.x = calculateDir(dir.x, mExtraOffset.x);
	dir.y = calculateDir(dir.y, mExtraOffset.y);

	const float speedX = calculateSpeed(dir.x, mExtraOffset.x, maxOffset.x);
	const float speedY = calculateSpeed(dir.y, mExtraOffset.y, maxOffset.y);



	auto moveOffset = [&](float& offset, float maxOffset, float dir, float speed) {
		const float movement = (dir * speed) * DeltaTime();

		if (fabs(offset) < maxOffset || Math::Sign(offset) != Math::Sign(dir)) {
			offset += movement;
		}
		};

	moveOffset(mExtraOffset.x, maxOffset.x, dir.x, speedX);
	moveOffset(mExtraOffset.y, maxOffset.y, dir.y, speedY);

	mExtraOffset.x = std::clamp(mExtraOffset.x, -mMaxOffset.x, mMaxOffset.x);
	mExtraOffset.y = std::clamp(mExtraOffset.y, -mMaxOffset.y, mMaxOffset.y);
}

void Script_MainCamera::Init()
{
	constexpr float maxPlaneDistance = 100.f;
	SetCameraOffset(Vec3(0.f, 12.f, -7.f));
	mObject->SetPosition(mPlayer->GetPosition() + mMainOffset);
	LookPlayer();

	MAIN_CAMERA->SetProjMtx(0.01f, maxPlaneDistance, 60.f);
}


void Script_MainCamera::LookPlayer()
{
	if (mPlayer) {
		MAIN_CAMERA->LookAt(mPlayer->GetPosition(), mPlayer->GetUp());
	}
}