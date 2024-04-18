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

	// [weight]      : �� ���⿡ �󸶳� ����ġ�� �� ���ΰ�(0~1)
	// [maxOffset_t] : [mMaxOffset]�� �󸶳� �ݿ��� ���ΰ� (0~1)
	// [isAlign]     : ������ ���� ��� �߾����� �̵��Ѵ�.
	void Move(Vec2 dir, Vec2 weight, float maxOffset_t = 1.f, bool isAlign = false);

private:
	void Init();
	// �÷��̾ �ٶ󺸵��� �Ѵ�.
	void LookPlayer();
};
#pragma endregion