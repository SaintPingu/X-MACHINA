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
	Vec3				mExtraOffset{};

	float				mSpeed{};

public:
	void SetCameraOffset(const Vec3& offset);

public:
	virtual void Start() override;
	virtual void Update() override;

private:
	void Init();
	// 플레이어를 바라보도록 한다.
	void LookPlayer();
};
#pragma endregion