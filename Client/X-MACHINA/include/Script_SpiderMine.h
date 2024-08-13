#pragma once


#pragma region Include
#include "Script_ExplosiveBullet.h"
#pragma endregion


#pragma region ClassForwardDecl
#pragma endregion

class Object;

#pragma region Class
class Script_SpiderMine : public Script_ExplosiveBullet {
	COMPONENT(Script_SpiderMine, Script_ExplosiveBullet)

private:
	float mMass{};
	float mDrag{};
	float mRotationSpeed{};
	float mUpSpeed{};
	float mPlantY{};
	bool mIsPlanted{};

public:
	virtual void Awake() override;
	virtual void Update() override;
	virtual void OnCollisionEnter(Object& other) override;

public:
	void Fire(const Vec3& pos, const Vec3& dir);

protected:
	virtual void StartFire() override;

private:
	void Move();
	void Plant();
};
#pragma endregion
