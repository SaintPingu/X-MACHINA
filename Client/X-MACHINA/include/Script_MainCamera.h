#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
#pragma endregion


#pragma region Class
// ���� �����ϴ� �� �ϳ��� ���� ī�޶� ��ũ��Ʈ
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

	// [weight]      : �� ���⿡ �󸶳� ����ġ�� �� ���ΰ�(0~1)
	// [maxOffset_t] : [mMaxOffset]�� �󸶳� �ݿ��� ���ΰ� (0~1)
	// [isAlign]     : ������ ���� ��� �߾����� �̵��Ѵ�.
	void Move(Vec2 dir, Vec2 weight, float maxOffset_t = 1.f);

	void ZoomIn();
	void ZoomOut();
	void ZoomReset() { mZoomAmount = 1.f; }

	void StartShake(float shakeTime, float amount = 0.002f) { mCrntShakeTime = shakeTime; mShakeAmount = amount; }

private:
	void Init();
	// �÷��̾ �ٶ󺸵��� �Ѵ�.
	void LookTarget();

	// õõ�� �߾��� �ٶ󺸵��� �Ѵ�.
	void RecoverExtraOffset();

	void Shake();

	void HideObstacles();
};
#pragma endregion