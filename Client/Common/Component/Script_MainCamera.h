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
	Vec3				mOffset = Vec3(0.f, 15, -30.f);

	float				mTimeLag{};							// translate 지연시간 (자연스러운 움직임)

public:
	void SetCameraOffset(const Vec3& offset);

public:
	virtual void Start() override;
	virtual void Update() override;

public:
	// 카메라가 지형 밑으로 내려가지 않도록 한다.
	void UpdateHeight();
	void Init();

private:
	// 플레이어를 바라보도록 한다.
	void LookPlayer();
};
#pragma endregion