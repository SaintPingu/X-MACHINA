#pragma once


#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
#pragma endregion


#pragma region Class
class Script_MainCamera : public Component {
	COMPONENT(Script_MainCamera, Component)

private:
	CameraMode			mCameraMode{ CameraMode::None };
	sptr<GameObject>	mPlayer{};
	Vec3				mOffset{ Vec3(0.f, 15, -30.f) };
	float				mTimeLag{};

public:
	void SetCameraOffset(const Vec3& offset);

public:
	virtual void Start() override;
	virtual void Update() override;

public:
	void UpdateHeight();
	void RotateOffset(const Vec3& axis, float angle);
	void ChangeCameraMode(CameraMode mode);

private:
	void LookPlayer();
};
#pragma endregion