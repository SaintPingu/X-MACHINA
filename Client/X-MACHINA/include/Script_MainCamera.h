#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
class GridObject;
#pragma endregion


#pragma region Class
// ���� �����ϴ� �� �ϳ��� ���� ī�޶� ��ũ��Ʈ
class Script_MainCamera : public Component {
	COMPONENT(Script_MainCamera, Component)

private:
	static constexpr float mkMaxZoomIn  = 0.25f;
	static constexpr float mkMaxZoomOut = 10.f;

	Vec3				mMainOffset{};
	Vec2				mExtraOffset{};
	Vec2				mMaxOffset{};

	float				mSpeed{};
	float				mZoomAmount{ 1.f };
	float				mShakeAmount{};
	bool				mIsMoved{};

	std::set<GridObject*> mHiddenBuildings{};

protected:
	GameObject* mTarget{};
	Vec3				mShakeOffset{};
	float				mCrntShakeTime{};

public:
	void SetCameraOffset(const Vec3& offset);
	virtual void InitCameraTarget();

public:
	virtual void Awake() override;
	virtual void OnEnable() override;
	virtual void Update() override;


	// [weight]      : �� ���⿡ �󸶳� ����ġ�� �� ���ΰ�(0~1)
	// [maxOffset_t] : [mMaxOffset]�� �󸶳� �ݿ��� ���ΰ� (0~1)
	// [isAlign]     : ������ ���� ��� �߾����� �̵��Ѵ�.
	void Move(Vec2 dir, Vec2 weight, float maxOffset_t = 1.f);

	void ZoomIn();
	void ZoomOut();
	void ZoomReset() { mZoomAmount = 1.f; }

	void StartShake(float shakeTime, float amount = 0.002f) { mCrntShakeTime = shakeTime; mShakeAmount = amount; }

protected:
	// Ÿ���� �ٶ󺸵��� �Ѵ�.
	virtual void LookTarget();

private:
	void Init();

	// õõ�� �߾��� �ٶ󺸵��� �Ѵ�.
	void RecoverExtraOffset();

	void Shake();
};
#pragma endregion