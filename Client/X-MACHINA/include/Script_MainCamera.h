#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
#pragma endregion


#pragma region Class
// 씬에 존재하는 단 하나의 메인 카메라 스크립트
class Script_MainCamera : public Component {
	COMPONENT(Script_MainCamera, Component)

private:
	static constexpr float mkMaxZoomIn  = 0.25f;
	static constexpr float mkMaxZoomOut = 10.f;

	GameObject*			mTarget{};
	Vec3				mMainOffset{};
	Vec2				mExtraOffset{};
	Vec2				mMaxOffset{};
	Vec3				mShakeOffset{};

	float				mSpeed{};
	float				mZoomAmount{ 1.f };
	float				mCrntShakeTime{};
	float				mShakeAmount{};
	bool				mIsMoved{};

public:
	void SetCameraOffset(const Vec3& offset);
	void SetCameraTarget(GameObject* target);

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void LateUpdate() override;

	// [weight]      : 각 방향에 얼마나 가중치를 둘 것인가(0~1)
	// [maxOffset_t] : [mMaxOffset]을 얼마나 반영할 것인가 (0~1)
	// [isAlign]     : 방향이 없는 경우 중앙으로 이동한다.
	void Move(Vec2 dir, Vec2 weight, float maxOffset_t = 1.f);

	void ZoomIn();
	void ZoomOut();
	void ZoomReset() { mZoomAmount = 1.f; }

	void StartShake(float shakeTime, float amount = 0.002f) { mCrntShakeTime = shakeTime; mShakeAmount = amount; }

private:
	void Init();
	// 플레이어를 바라보도록 한다.
	void LookTarget();

	// 천천히 중앙을 바라보도록 한다.
	void RecoverExtraOffset();

	void Shake();

	void HideObstacles();
};
#pragma endregion