#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class Script_PlayerController;
class InstObject;
class ParticleSystem;
class Script_LiveObject;
#pragma endregion


#pragma region Class
class Script_Bullet : public Component {
	COMPONENT(Script_Bullet, Component)

private:
	Vec3 mBeforePos{};
	InstObject*		mInstObject{};	// self GameObject
	BulletPSType    mParticleType{};
	std::string		mImpactSound{};

	std::array<std::vector<std::string>, BulletPSTypeCount> mPSNames;
	std::array<std::queue<ParticleSystem*>, BulletPSTypeCount> mPSs;

	float mDamage{};		// damage of bullet
	float mCurDistance{};	// 현재 비행 길이
	float mMaxDistance{};	// 최대 비행 길이

	float mEndDistance{ 40.f };	// 비행 종료 길이

	bool mIsPlayerBullet{};

protected:
	Script_PlayerController* mPlayerController{};
	float mSpeed{};			// speed of bullet

public:
	float GetDamage() const { return mDamage; }
	bool IsPlayerBullet() const { return mIsPlayerBullet; }

	void SetDamage(float damage) { mDamage = damage; }
	void SetSpeed(float speed) { mSpeed = speed; }
	void SetEndDistance(float distance) { mEndDistance = distance; }
	void SetParticleSystems(BulletPSType type, const std::vector<std::string>& psNames);
	void SetImpactSound(const std::string& sound) { mImpactSound = sound; }
	void SetPlayerBullet(bool val, const Object* player);

public:
	virtual void Update() override;

public:
	virtual void Init();

	// [pos] 위치에 생성하고 [dir, up]에 따라 look 방향을 결정하고, look 방향으로 [speed]의 속도로 이동하도록 한다.
	virtual void Fire(const Vec3& pos, const Vec3& dir);
	// [err] 만큼 각도로 탄이 퍼진다.
	void Fire(const Transform& transform, const Vec2& err = Vector2::Zero);

	virtual void Explode();
	void PlayPSs(BulletPSType type);
	void StopPSs(BulletPSType type); 

protected:
	virtual void StartFire() {}
	void Hit(Script_LiveObject* target);
	Vec3 ApplyErr(const Vec3& dir, const Vec2& err);

private:
	void Disappear();
	void Reset();
};
#pragma endregion
