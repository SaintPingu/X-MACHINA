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
	static constexpr float mkMaxZoomOut = 2.f;

	sptr<GameObject>	mTarget{};
	Vec3				mMainOffset{};
	Vec2				mExtraOffset{};
	Vec2				mMaxOffset{};

	float				mSpeed{};
	float				mZoomAmount{ 1.f };
	bool				mIsMoved{};

public:
	void SetCameraOffset(const Vec3& offset);
	void SetCameraTarget(sptr<GameObject> target);

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

	// [weight]      : �� ���⿡ �󸶳� ����ġ�� �� ���ΰ�(0~1)
	// [maxOffset_t] : [mMaxOffset]�� �󸶳� �ݿ��� ���ΰ� (0~1)
	// [isAlign]     : ������ ���� ��� �߾����� �̵��Ѵ�.
	void Move(Vec2 dir, Vec2 weight, float maxOffset_t = 1.f);

	void ZoomIn();
	void ZoomOut();
	void ZoomReset() { mZoomAmount = 1.f; }

private:
	void Init();
	// �÷��̾ �ٶ󺸵��� �Ѵ�.
	void LookTarget();

	// õõ�� �߾��� �ٶ󺸵��� �Ѵ�.
	void RecoverExtraOffset();
};
#pragma endregion