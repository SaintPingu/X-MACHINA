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

	RecoverExtraOffset();
}

void Script_MainCamera::Move(Vec2 dir, Vec2 weight, float maxOffset_t)
{
	maxOffset_t = std::clamp(maxOffset_t, 0.f, 1.f);
	const Vec2 maxOffset = mMaxOffset * maxOffset_t;

	constexpr float originSpeed = 1.f;

	auto CalculateDir = [&](float dir, float extraOffset) -> float {
		if (Math::IsZero(dir)) {
			return 0.f;
		}
		else {
			return static_cast<float>(Math::Sign(dir));
		}
		};

	auto CalculateSpeed = [&](float dir, float extraOffset, float maxOffset) -> float {
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

	dir.x = CalculateDir(dir.x, mExtraOffset.x);
	dir.y = CalculateDir(dir.y, mExtraOffset.y);

	weight.x = std::clamp(weight.x, 0.f, 1.f);
	weight.y = std::clamp(weight.y, 0.f, 1.f);

	const float speedX = CalculateSpeed(dir.x, mExtraOffset.x, maxOffset.x) * weight.x;
	const float speedY = CalculateSpeed(dir.y, mExtraOffset.y, maxOffset.y) * weight.y;


	auto MoveOffset = [&](float& offset, float maxOffset, float dir, float speed) {
		const float movement = (dir * speed) * DeltaTime();

		// 최대치보다 작거나 반대방향 이동 시 speed 그대로 적용
		if (fabs(offset) < maxOffset || Math::Sign(offset) != Math::Sign(dir)) {
			offset += movement;
		}
		// maxOffset을 넘어선 경우 maxOffset까지 도달하게 한다.
		else {
			offset -= movement;
			if (fabs(offset) < maxOffset) {
				offset = maxOffset * Math::Sign(offset);
			}
		}
		};

	MoveOffset(mExtraOffset.x, maxOffset.x, dir.x, speedX);
	MoveOffset(mExtraOffset.y, maxOffset.y, dir.y, speedY);

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

void Script_MainCamera::RecoverExtraOffset()
{
	auto Recover = [](float& offset) {
		constexpr float speedDecDistance = 1.f;
		float recoverSpeed = 0.3f;
		// 거리가 [speedDecDistance] 미만일 때, 0에 근접할 수록(offset에 비례) 속도를 감소한다.
		if (fabs(offset) < speedDecDistance) {
			recoverSpeed *= fabs(offset) / speedDecDistance;
		}
		if (fabs(offset) > 0.1f) {
			offset -= Math::Sign(offset) * recoverSpeed * DeltaTime();
		}
		};

	Recover(mExtraOffset.x);
	Recover(mExtraOffset.y);
}
