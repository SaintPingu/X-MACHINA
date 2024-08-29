#pragma once


#pragma region Include
#include "Script_MainCamera.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
class GridObject;
#pragma endregion


#pragma region Class
// 씬에 존재하는 단 하나의 메인 카메라 스크립트
class Script_CinematicCamera : public Script_MainCamera {
	COMPONENT(Script_CinematicCamera, Script_MainCamera)

private:
	float mSpeed{};
	float mRotSpeed{};
	Vec3 mOrigin{};
	bool mIsPlaying{};

public:
	virtual void InitCameraTarget() override;
	virtual void Start() override;
	virtual void Update() override;
};
#pragma endregion