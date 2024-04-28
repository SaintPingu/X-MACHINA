#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class InstObject;
class Rigidbody;
class ParticleSystem;
#pragma endregion


#pragma region Class
class Script_Bullet : public Component {
	COMPONENT(Script_Bullet, Component)

private:
	InstObject*		mGameObject{};	// self GameObject
	const Object*	mOwner{};		// 총알을 발사한 객체 (자신은 충돌하지 않도록 한다)
	sptr<ParticleSystem> mParticleSystem{};
	sptr<Rigidbody> mRigid{};

	std::array<std::vector<sptr<ParticleSystem>>, BulletPSTypeCount> mPSs;

	float mSpeed{};					// speed of bullet
	float mDamage{};				// damage of bullet
	float mMaxLifeTime{ 2.f };		// 총알 수명(발사 후 최대 유지시간)
	float mCurrLifeTime{};			// 현재 수명

public:
	float GetDamage() const { return mDamage; }

	void SetDamage(float damage) { mDamage = damage; }
	void SetSpeed(float speed) { mSpeed = speed; }
	void SetOwner(const Object* object) { mOwner = object; }
	void SetLifeTime(float lifeTIme) { mMaxLifeTime = lifeTIme; }
	void SetParticleSystems(BulletPSType type, const std::vector<std::string>& psNames);

public:
	virtual void Update() override;

	virtual void OnCollisionStay(Object& other) override;

public:
	virtual void Init();

	// [pos] 위치에 생성하고 [dir, up]에 따라 look 방향을 결정하고, look 방향으로 [speed]의 속도로 이동하도록 한다.
	void Fire(const Vec3& pos, const Vec3& dir, const Vec3& up);
	// [err] 만큼 각도로 탄이 퍼진다.
	void Fire(const Transform& transform, const Vec2& err = Vector2::Zero);

	// 총알 객체를 터뜨린다. (폭발 처리)
	virtual void Explode();
	void PlayPSs(BulletPSType type); 
	void StopPSs(BulletPSType type); 
	void ResetPSs(BulletPSType type); 

protected:
	virtual void StartFire() {};

private:
	bool IsOwner(const Object* object) { return mOwner == object; }
	void Reset();
	// terrain과 충돌 여부를 검사한다.
	bool IntersectTerrain();
};
#pragma endregion
