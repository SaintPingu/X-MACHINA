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
	sptr<GameObject>	mPlayer{};
	Vec3				mMainOffset{};
	Vec2				mExtraOffset{};
	Vec2				mMaxOffset{};

	float				mSpeed{};

public:
	void SetCameraOffset(const Vec3& offset);

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

	// [weight]      : 각 방향에 얼마나 가중치를 둘 것인가(0~1)
	// [maxOffset_t] : [mMaxOffset]을 얼마나 반영할 것인가 (0~1)
	// [isAlign]     : 방향이 없는 경우 중앙으로 이동한다.
	void Move(Vec2 dir, Vec2 weight, float maxOffset_t = 1.f, bool isAlign = false);

private:
	void Init();
	// 플레이어를 바라보도록 한다.
	void LookPlayer();
};
#pragma endregion