#pragma once
#undef max
#include "Script_NetworkLiveObject.h"
#include "ClientNetwork/Include/Protocol/Enum_generated.h"

/* Extrapolation Data - 패킷을 통한 위치 예측을 위해 필요한 데이터 */

class Script_Weapon;
class Transform;
class GridObject;
class AnimatorController;
class AnimatorMotion;
class Airstrike;
class Script_Remote_Ability_Shield;
class Script_Remote_Ability_Cloaking;
class Script_Remote_Ability_MindControl;
class Script_BattleUI;


/// +-------------------------------------------------
///		Extrapolate Data 
/// -------------------------------------------------+
struct AnimMoveData
{
	float AnimParam_h = {};
	float AnimParam_v = {};
};


/// +-------------------------------------------------
///		Extrapolate Data 
/// -------------------------------------------------+
struct ExtData
{
	enum class MOVESTATE {
		Start    = 0,
		Progress = 1,
		End      = 2,
		Default  = 3,
		NONE,
	};	

	long long		PingTime		= {}; // 다음 Transform 패킷이 오기까지 걸리는 시간 ( 추측 - ( SPkt_Transform Recv Interval + Remote Client Latency + My Latency )
	MOVESTATE		MoveState		= MOVESTATE::NONE;
	float			Velocity		= {};
	Vec3			TargetPos		= {};
	Vec3			TargetRot		= {};
	Vec3			MoveDir			= {};

	AnimMoveData	Animdata		= {};
};


class Script_NetworkRemotePlayer : public Script_NetworkLiveObject
{
	COMPONENT(Script_NetworkRemotePlayer, Script_NetworkLiveObject)

private:
	/// +-------------------------------------------------
	///		TRANSFORM ( Dead Reckoning )
	/// -------------------------------------------------+
	ExtData				mCurrExtraPolated_Data = {};
	ExtData				mPrevExtrapolated_Data = {};

	Vec3				mCurrMoveDir = Vec3();
	Vec3				mPrevMoveDir = Vec3();
	Vec3				mCurrPacketPos = Vec3();
	Vec3				mPrevPacketPos = Vec3();

	ExtData::MOVESTATE	mMoveState;
	float				mRotationSpeed = 360.f;
	float				mLerpVal = 0.f;
	double				mBezierTime = 0.f;
	int					mFireCnt{};	// 패킷 밀릴 경우 한 번 씩 발사하기 위함

	Transform* mSpine{};
	float mSpineAngle{};
	AnimatorController* mController;
	bool mIsAim{};
	float mParamV{};
	float mParamH{};

	std::unordered_map<WeaponName, GridObject*> mWeapons{};
	std::unordered_map<GridObject*, sptr<Script_Weapon>> mWeaponScripts{};
	GridObject* mCrntWeapon{};
	sptr<Airstrike> mAirstrike{};

	sptr<Script_Remote_Ability_Shield> mRemoteAbilityShield{};
	sptr<Script_Remote_Ability_Cloaking> mRemoteAbilityCloaking{};
	sptr<Script_Remote_Ability_MindControl> mRemoteAbilityMindControl{};
	std::unordered_map<int, AnimatorMotion*> mReloadMotions;
	bool mIsReloading{};

	/// +-------------------------------------------------
	///		WEAPON 
	/// -------------------------------------------------+
	WeaponName	       mCurrWeaponName			= {};

	Script_BattleUI* mBattleUI{};
	std::wstring mName{ L"Remote_Unknown" };
	int mLevel{};
	
public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void OnDestroy() override;
	virtual void LateUpdate() override;
	virtual void UpdateData(const void* data) override;

public:
	const std::wstring& GetName() const { return mName; }
	int GetLevel() const { return mLevel; }

	void SetName(const std::wstring& name) { mName = name; }
	void SetLevel(int level) { mLevel = level; }

	/// +-------------------------------------------------
	///		Extrapolate 
	/// -------------------------------------------------+
public:
	void MoveToTargetPos(Vec3 TargetPos);
	void MoveToPacketPos(Vec3 PacketPos);
	void SetExtrapolatedData(ExtData& extData);
	void SetExtrapolated_Pos(Vec3 pos)									{ mCurrExtraPolated_Data.TargetPos      = pos;									 }	
	void SetExtrapolated_Rot(Vec3 rot)									{ mCurrExtraPolated_Data.TargetRot      = rot;									 }
	void SetExtrapolated_PingTime(long long pingTime)					{ mCurrExtraPolated_Data.PingTime       = pingTime;								 }
	void SetExtrapolated_MoveState(ExtData::MOVESTATE movestate)		{ mCurrExtraPolated_Data.MoveState      = movestate;							 }
	void SetPacketPos(Vec3 pos)											{ mPrevPacketPos = mCurrPacketPos;  mCurrPacketPos = pos; mBezierTime = 0.f;  }

	/// +-------------------------------------------------
	///		Extrapolate Util Func 
	/// -------------------------------------------------+
public:
	float GetYAngle();
	Vec3  GetDirection(Vec3 dir);
	Vec3  CalculateDirection(float yAngleRadian);
	Vec3  lerp(Vec3 CurrPos, Vec3 TargetPos, float PosLerpParam);
	Vec3  lerp(Vec3 CurrPos, Vec3 TargetPos, float Deltatime, float speed);
	Vec3  quadraticInterpolation(const Vec3& p0, const Vec3& p1, const Vec3& p2, float t);
	float GetYAngleFromQuaternion(const Vec4& rotationQuaternion);
	void RotateTo(float yAngle, float ySpineAngle);
	void RotateTo(Transform* transform, const Vec3& dir, float speed);
	Vec3  Bezier_Curve_3(Vec3 start, Vec3 Target, float t);
	Vec3 Script_NetworkRemotePlayer::Bezier_Curve(Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, float t);
	float Distance(const Vec3& v1, const Vec3& v2);

	void UpdateParams();
	void UpdateParam(float val, float& param);

	void FireBullet(const Vec3& firePos, const Vec3& look);
	

	/// +-------------------------------------------------
	///		WEAPON 
	/// -------------------------------------------------+
public:
	void		SetCurrWeaponName(FBProtocol::ITEM_TYPE weaponType);
	WeaponName	GetCurrWeaponName() { return mCurrWeaponName; }
	void TakeWeapon(rsptr<Script_Weapon> weapon);
	void DropWeapon(WeaponName weaponName);
	void SetWeapon(WeaponName weaponName);
	void SetWeaponChild(GridObject* weapon);
	
public:
	void ToggleAbilityShield();
	void ToggleAbilityCloaking();
	void ToggleAbilityMindControl(Object* pickedTarget);
	void EndReloadCallback();
	void StartReloadCallback();

private:
	void Script_NetworkRemotePlayer::ResetBoltActionMotionSpeed(rsptr<Script_Weapon> weapon);
	void Script_NetworkRemotePlayer::SetMotionSpeed(AnimatorMotion* motion, float time);
};

