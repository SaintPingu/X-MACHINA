#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
class Animator;
class AnimatorMotion;
class AnimatorController;
class Script_Weapon;
class Script_AimController;
class Script_MainCamera;
#pragma endregion


#pragma region EnumClass
// 플레이어 객체 타입
enum class PlayerType {
	Unspecified = 0,
	Tank,
	Airplane,
	Human
};

class Movement : public DwordOverloader<Movement> {
	DWORD_OVERLOADER(Movement)

	static const DWORD None   = 0x00;
	static const DWORD Stand  = 0x01;
	static const DWORD Sit    = 0x02;
	static const DWORD Walk   = 0x10;
	static const DWORD Run    = 0x20;
	static const DWORD Sprint = 0x40;

	static Movement GetState(Movement movement)  { return movement & 0x0F; }
	static Movement GetMotion(Movement movement) { return movement & 0xF0; }
};
#pragma endregion


#pragma region Class
class Script_Player abstract : public Component {
	COMPONENT_ABSTRACT(Script_Player, Component)

protected:
	Script_MainCamera* mCamera{};

	PlayerType		mPlayerType{};
	GameObject*		mPlayer{};		// self GameObject
	Matrix			mSpawnTransform{};	// 리스폰 지점

	int		mScore{};
	float	mMaxHP{};
	float	mHP{};

public:
	PlayerType GetPlayerType() const { return mPlayerType; }

	void SetHP(float hp) { mMaxHP = hp; mHP = hp; }
	// player를 [pos]로 위치시키고 해당 위치를 리스폰 지점으로 설정한다.
	void SetSpawn(const Vec3& pos);

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual void ProcessInput() {}
	virtual void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam) {}
	virtual void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam) {}

	virtual void Rotate(float pitch, float yaw, float roll);

	void Respawn();
	void Explode();
	void Hit(float damage);
	void AddScore(int score);
};





// 총알을 발사할 수 있는 플레이어
class Script_ShootingPlayer abstract : public Script_Player {
	COMPONENT_ABSTRACT(Script_ShootingPlayer, Script_Player)

private:
	int mCrntWeaponIdx{};
	int mNextWeaponIdx{};
	bool mIsInDraw{};
	bool mIsInPutback{};

protected:
	sptr<GameObject> mWeapon{};
	sptr<Script_Weapon> mWeaponScript{};
	std::vector<sptr<GameObject>> mWeapons{};
	Transform* mMuzzle{};

public:
	virtual void Start() { base::Start();  InitWeapons(); }

public:
	virtual void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
	virtual void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam);

	bool IsInGunChangeMotion() const { return IsInDraw() || IsInPutBack(); }
	bool IsInDraw() const { return mIsInDraw; }
	bool IsInPutBack() const { return mIsInPutback; }

	virtual void BulletFired() {}

protected:
	int GetCrntWeaponIdx() const { return mCrntWeaponIdx; }
	int GetNextWeaponIdx() const { return mNextWeaponIdx; }

	virtual void InitWeapons() abstract;

	virtual void SetWeapon(int weaponIdx);

	virtual void DrawWeaponStart(int weaponIdx, bool isDrawImmed) abstract;
	virtual void DrawWeapon();
	virtual void DrawWeaponEnd();
	virtual void PutbackWeapon() abstract;
	virtual void PutbackWeaponEnd();

	virtual void StartFire();
	virtual void StopFire();
	virtual bool Reload();
};




// 지상 플레이어
class Script_GroundPlayer : public Script_ShootingPlayer {
	COMPONENT(Script_GroundPlayer, Script_ShootingPlayer)

private:
	// constants //
	static const float mkSitWalkSpeed;
	static const float mkStandWalkSpeed;
	static const float mkRunSpeed;
	static const float mkSprintSpeed;

	static const float mkStartRotAngle;

	// animator //
	float mParamV{}, mParamH{};
	sptr<Animator> mAnimator{};
	sptr<AnimatorController> mController{};

	std::unordered_map<int, sptr<AnimatorMotion>> mReloadMotions;

	// movement //
	Movement mPrevMovement{};
	float mMovementSpeed{};
	float mRotationSpeed{};

	// aim //
	bool mIsAim{};
	bool mIsInBodyRotation{};
	float mCrntSpineAngle{};
	float mSpineDstAngle{};
	float mAimingDeltaTime{};
	float mReloadingDeltaTime{};
	float mCrntYawAngle{};
	Transform* mSpineBone{};
	sptr<Script_AimController> mAimController{};

	// recoil //
	int   mRecoilSign{};
	float mCurRecoil{};
	float mMaxRecoil{20.f};

public:
	Movement GetPrevState() const  { return Movement::GetState(mPrevMovement); }
	Movement GetPrevMotion() const { return Movement::GetMotion(mPrevMovement); }

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void LateUpdate() override;

	virtual void OnCollisionStay(Object& other) override;

public:
	void UpdateParams(Dir dir, float v, float h, float rotAngle);
	virtual void ProcessInput() override;

	// direction 방향으로 이동한다.
	virtual void Move(Dir dir);
	// [dir]방향을 바라보도록 회전한다.
	void RotateTo(Dir dir);

	virtual void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam) override;
	virtual void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam) override;

	// called by weapon //
	void StartReload();
	void BoltAction();
	// called by callback //
	void EndReload();

	virtual void BulletFired() override;

private:
	void InitWeapons();
	virtual void DrawWeaponStart(int weaponIdx, bool isDrawImmed) override;
	virtual void DrawWeaponCallback();
	virtual void DrawWeaponEndCallback();
	virtual void PutbackWeapon() override;
	virtual void PutbackWeaponEndCallback();
	void UpdateParam(float val, float& param);

	void UpdateMovement(Dir dir);

	float GetAngleMuzzleToAim(const Vec3& aimWorldPos) const;
	float GetAngleSpineToAim(const Vec3& aimWorldPos) const;
	Vec3 GetAimWorldPos(const Vec2& aimScreenPos) const;
	void RotateToAim(Dir dir, float& rotAngle);

	// angle 만큼 서서히 회전한다.
	void Rotate(float angle) const;
	void RotateMuzzleToAim();

	void OnAim();
	void OffAim();

	// called by itself
	virtual bool Reload() override;
	void StopReload();

	void SetState(Movement prevState, Movement prevMotion, Movement crntState);
	void SetMotion(Movement prevState, Movement prevMotion, Movement crntState, Movement& crntMotion);

	void StopReloadCallback();
	void ChangeReloadCallback();
	void EndReloadCallback();
	void BoltActionCallback();

	void ResetAimingTime() { mAimingDeltaTime = 0.f; }
	void RecoverRecoil();

	void MoveCamera(Dir dir);

	// [time] 내에 [motion]이 끝나도록 [motion]의 속도를 변경한다.
	void SetMotionSpeed(rsptr<AnimatorMotion> motion, float time);
};

#pragma endregion