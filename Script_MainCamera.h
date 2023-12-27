#pragma once
#include "Component.h"

class Camera;
class CMainCamera;
class CGameObject;


class Script_MainCamera : public Component {
	COMPONENT(Component, Script_MainCamera)

private:
	Camera* mCamera{};
	CMainCamera* mCameraObject{};
	CGameObject* mPlayer{};
	Vec3 mOffset = Vec3(0.0f, 15, -30.0f);
	float mTimeLag{};

	CameraMode mCameraMode{ CameraMode::None };

public:
	virtual void Start() override;
	virtual void Update() override;
	void UpdateHeight();

	void SetCameraOffset(const Vec3& offset);
	void RotateOffset(const Vec3& axis, float angle);

	void ChangeCameraMode(CameraMode mode);
};
