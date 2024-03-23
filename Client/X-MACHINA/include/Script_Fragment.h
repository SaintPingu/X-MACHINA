#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion




#pragma region Class
// 폭발 시 발생하는 파편(single object)
class Script_Fragment : public Component {
	COMPONENT(Script_Fragment, Component)

private:
	Vec3 mRotationAxis{ Vector3::Up() };
	Vec3 mMovingDir{};
	float mMovingSpeed{};
	float mRotationSpeed{};

public:
	void SetRotationAxis(const Vec3& rotationAxis) { mRotationAxis = Vector3::Normalize(rotationAxis); }
	void SetRotationSpeed(float speed) { mRotationSpeed = speed; }
	void SetMovingSpeed(float speed) { mMovingSpeed = speed; }
	void SetMovingDir(const Vec3& dir) { mMovingDir = dir; }

public:
	virtual void Awake() override;
	virtual void Update() override;

public:
	// create fragment at [pos]
	void Active(const Vec3& pos);
};
#pragma endregion