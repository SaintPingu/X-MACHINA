#pragma once


#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
#pragma endregion


#pragma region Class
// ���� �����ϴ� �� �ϳ��� ���� ī�޶� ��ũ��Ʈ
class Script_MainCamera : public Component {
	COMPONENT(Script_MainCamera, Component)

private:
	CameraMode			mCameraMode{ CameraMode::None };
	sptr<GameObject>	mPlayer{};
	Vec3				mOffset = Vec3(0.f, 15, -30.f);

	float				mTimeLag{};							// translate �����ð� (�ڿ������� ������)

public:
	void SetCameraOffset(const Vec3& offset);

public:
	virtual void Start() override;
	virtual void Update() override;

public:
	// ī�޶� ���� ������ �������� �ʵ��� �Ѵ�.
	void UpdateHeight();
	void ChangeCameraMode(CameraMode mode);

private:
	// �÷��̾ �ٶ󺸵��� �Ѵ�.
	void LookPlayer();
};
#pragma endregion