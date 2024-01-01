#pragma once


#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
class Rigidbody;
#pragma endregion


#pragma region Class
// 총알 객체
// Fire 함수를 통해 active한다.
class Script_Bullet : public Component {
	COMPONENT(Script_Bullet, Component)

private:
	GameObject*		mGameObject{};	// self GameObject
	sptr<Rigidbody> mRigid{};
	const Object*	mOwner{};		// 총알을 발사한 객체 (자신은 충돌하지 않도록 한다)

	float mDamage{};				// damage of bullet
	float mMaxLifeTime{ 1000.f };	// 총알 수명(발사 후 최대 유지시간)
	float mCurrLifeTime{};			// 현재 수명

public:
	float GetDamage() { return mDamage; }

	void SetDamage(float damage) { mDamage = damage; }
	void SetOwner(const Object* object) { mOwner = object; }
	void SetLifeTime(float lifeTIme) { mMaxLifeTime = lifeTIme; }

public:
	virtual void Start() override;
	virtual void Update() override;

	virtual void OnCollisionStay(Object& other) override;

public:
	// [pos] 위치에 생성하고 [dir, up]에 따라 look 방향을 결정하고, look 방향으로 [speed]의 속도로 이동하도록 한다.
	void Fire(const Vec3& pos, const Vec3& dir, const Vec3& up, float speed);
	// 총알 객체를 터뜨린다. (폭발 처리)
	void Explode();

private:
	bool IsOwner(const Object* object) { return mOwner == object; }
	void Reset();
	// terrain과 충돌 여부를 검사한다.
	bool IntersectTerrain();
};
#pragma endregion
