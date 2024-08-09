#pragma once


#pragma region Include
#include "Script_LiveObject.h"
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class Animator;
class AnimatorMotion;
class AnimatorController;
class Script_Weapon;
class Script_MainCamera;
class Script_BattleUI;
class GridObject;
#pragma endregion


#pragma region Class
class Script_Player abstract : public Script_LiveObject {
	COMPONENT_ABSTRACT(Script_Player, Script_LiveObject)

private:
	int mLevel{};
	std::wstring mName{ L"Unknown" };

	Matrix			mSpawnTransform{};	// 리스폰 지점
	bool mIsInteracted{};

protected:
	Script_MainCamera* mCamera{};

public:
	virtual void Start() override;
	virtual void Update() override;

public:
	const std::wstring& GetName() const { return mName; }
	int GetLevel() const { return mLevel; }

	void SetName(const std::wstring& name) { mName = name; }
	void SetLevel(int level) { mLevel = level; }

	// player를 [pos]로 위치시키고 해당 위치를 리스폰 지점으로 설정한다.
	void SetSpawn(const Vec3& pos);

public:
	virtual bool ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam) { return true; }
	virtual bool ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam);

	virtual void Rotate(float pitch, float yaw, float roll);

	virtual void Dead() override;
	virtual void Respawn();

private:
	virtual void Interact();
};





// 총알을 발사할 수 있는 플레이어
class Script_ShootingPlayer abstract : public Script_Player {
	COMPONENT_ABSTRACT(Script_ShootingPlayer, Script_Player)

private:
	int mCrntWeaponNum{};
	int mNextWeaponNum{};
	bool mIsInDraw{};
	bool mIsInPutback{};
	int mPlayerIdx{};

protected:
	Script_BattleUI* mBattleUI{};
	GridObject* mWeapon{};
	sptr<Script_Weapon> mWeaponScript{};
	std::vector<GridObject*> mWeapons{};
	Transform* mMuzzle{};

public:
	virtual void Start() override;
	virtual void OnDestroy() override;

public:
	rsptr<Script_Weapon> GetCrntWeapon() const { return mWeaponScript; }

	bool IsInGunChangeMotion() const { return IsInDraw() || IsInPutBack(); }
	bool IsInDraw() const { return mIsInDraw; }
	bool IsInPutBack() const { return mIsInPutback; }

public:
	virtual void BulletFired();

	virtual bool ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam) override;
	virtual bool ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam) override;

	virtual void StartFire();
	virtual void StopFire();
	virtual bool Reload();

	virtual void DrawWeapon(int weaponNum);

	void RemoveWeaponUI() const;
	void UpdateWeaponUI() const;

protected:
	int GetCrntWeaponIdx() const { return mCrntWeaponNum - 1; }
	int GetCrntWeaponNum() const { return mCrntWeaponNum; }
	int GetNextWeaponNum() const { return mNextWeaponNum; }

	virtual void DrawWeaponStart(int weaponNum, bool isDrawImmed) abstract;
	virtual void DrawWeapon();
	virtual void DrawWeaponEnd();
	virtual void PutbackWeapon() abstract;
	virtual void PutbackWeaponEnd();
	virtual void DropWeapon(int weaponIdx);
	virtual void ResetWeaponAnimation() {}

private:
	void SetWeapon(int weaponNum);
	void ResetNextWeaponNum() { mNextWeaponNum = 0; }
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
	Animator* mAnimator{};
	AnimatorController* mController{};

	std::unordered_map<int, AnimatorMotion*> mReloadMotions;

	// movement //
	PlayerMotion mPrevMovement{};
	float mMovementSpeed{};
	float mRotationSpeed{};

	// aim //
	bool mIsAim{};
	bool mIsInBodyRotation{};	// is in spine rotation
	float mCrntSpineAngle{};
	float mSpineDstAngle{};
	float mAimingDeltaTime{};
	float mReloadingDeltaTime{};
	float mCrntYawAngle{};
	Transform* mSpineBone{};

	// recoil //
	int   mRecoilSign{};
	float mCurRecoil{};
	float mMaxRecoil{20.f};

	// sliding vector //
	Vec3 mDirVec{};
	Vec3 mPrevDirVec{};
	Vec3 mSlideVec{};

	// For Network
	Vec3 mPrevPos{};
	Vec3 mCurrPos{};
public:
	Vec3 GetPrevPos() { return mPrevPos; }
	Vec3 GetcurrPos() { return mCurrPos; }

public:
	PlayerMotion GetPrevState() const  { return PlayerMotion::GetState(mPrevMovement); }
	PlayerMotion GetPrevMotion() const { return PlayerMotion::GetMotion(mPrevMovement); }
	bool IsReloading() const;
	Transform* GetSpineBone() { return mSpineBone; }

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void LateUpdate() override;
	virtual void OnDestroy() override;

	virtual void OnCollisionStay(Object& other) override;

public:
	void UpdateParams(Dir dir, float v, float h, float rotAngle);
	void ProcessInput();
	virtual bool ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam) override;

	// direction 방향으로 이동한다.
	virtual void Move(Dir dir);
	// [dir]방향을 바라보도록 회전한다.
	void RotateTo(Dir dir);

	void InitWeaponAnimations();
	// called by weapon //
	void StartReload();
	void BoltAction();
	// called by callback //
	void EndReload();
	// called by itself //
	virtual bool Reload() override;

	virtual void BulletFired() override;

	float GetMovementSpeed() const { return mMovementSpeed; }
	float GetRotationSpeed() const { return mRotationSpeed; }

	void AquireNewWeapon(WeaponName weaponName);
	void TakeWeapon(rsptr<Script_Weapon> weapon);

	Vec3  GetMoveDir() const	   { return mDirVec; }
	Vec3  GetPrevMoveDir() const   { return mPrevDirVec; }

	void OnAim();
	void OffAim();

	virtual void DropCrntWeapon();

private:
	virtual void DrawWeaponStart(int weaponNum, bool isDrawImmed) override;
	virtual void DrawWeaponCallback();
	virtual void DrawWeaponEndCallback();
	virtual void PutbackWeapon() override;
	virtual void PutbackWeaponEndCallback();
	virtual void DropWeapon(int weaponIdx) override;
	void UpdateParam(float val, float& param);
	virtual void ResetWeaponAnimation() override;

	void UpdateMovement(Dir dir);

	float GetAngleMuzzleToAim(const Vec3& aimWorldPos) const;
	float GetAngleSpineToAim(const Vec3& aimWorldPos) const;
	Vec3 GetAimWorldPos(const Vec2& aimScreenPos) const;
	void RotateToAim(Dir dir, float& rotAngle);


	// angle 만큼 서서히 회전한다.
	void Rotate(float angle) const;
	void RotateMuzzleToAim();

	void StopReload();

	void SetState(PlayerMotion prevState, PlayerMotion prevMotion, PlayerMotion crntState);
	// Cancel the previous state and switch to the current state.
	void SetMotion(PlayerMotion prevState, PlayerMotion prevMotion, PlayerMotion crntState, PlayerMotion& crntMotion);

	void StopReloadCallback();
	void ChangeReloadCallback();
	void EndReloadCallback();
	void BoltActionCallback();
	void BoltActionSoundCallback();

	void ResetAimingTime() { mAimingDeltaTime = 0.f; }
	void RecoverRecoil();

	void MoveCamera(Dir dir);

	// [time] 내에 [motion]이 끝나도록 [motion]의 속도를 변경한다.
	void SetMotionSpeed(AnimatorMotion* motion, float time);
	void ComputeSlideVector(Object& other);

	void SwitchWeapon(GridObject* weapon);
	void SetWeaponChild(GridObject* weapon);

	void ResetBoltActionMotionSpeed(rsptr<Script_Weapon> weapon);
};


// 페로 능력을 가진 플레이어
class Script_PheroPlayer : public Script_GroundPlayer {
	COMPONENT(Script_PheroPlayer, Script_GroundPlayer)

protected:
	float mStartPheroAmount{};
	float mCurPheroAmount{};
	float mMaxPheroAmount{};
	float mPheroRegenRate{};
	float mPrevPheroAmount{};

public:
	virtual void Awake() override;
	virtual void Update() override;
	virtual void Respawn() override;

public:
	float GetMaxPheroAmount() const { return mMaxPheroAmount; }
	float GetCurPheroAmount() const { return mCurPheroAmount; }
	virtual void AddPheroAmount(float pheroAmount);
	virtual bool ReducePheroAmount(float pheroCost, bool checkOnly = false);
};
#pragma endregion