#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion

class Script_MainCamera;

class Script_AerialCamera : public Component {
	COMPONENT(Script_AerialCamera, Component)

private:
	wptr<Script_MainCamera> mMainCamera{};

	Vec3 mOffset{};
	float mMovementSpeed{};

	float mCurDist{};
	float mStartDist{};
	float mEndDist{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;

private:
	void Init();
	void SetCameraOffset(const Vec3& offset);

	void Move();
};