#include "stdafx.h"
#include "Script_NetworkRemotePlayer.h"

#include "Script_Weapon.h"
#include "Script_Weapon_Pistol.h"
#include "Script_Weapon_Rifle.h"
#include "Script_Weapon_Shotgun.h"
#include "Script_Weapon_Sniper.h"
#include "Script_Weapon_MissileLauncher.h"
#include "Script_Weapon_MineLauncher.h"
#include "Airstrike.h"
#include "Script_BattleManager.h"
#include "Script_BattleUI.h"

#include "Timer.h"
#include "Object.h"
#include "Animator.h"
#include "AnimatorController.h"
#include "AnimatorMotion.h"
#include "BattleScene.h"
#include "ClientNetwork/Include/LogManager.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"

#include "Script_Ability_Shield.h"
#include "Script_Ability_Cloaking.h"
#include "Script_Ability_MindControl.h"

void Script_NetworkRemotePlayer::Awake()
{
	static const std::unordered_map<WeaponType, std::string> kReloadMotions{
		{WeaponType::HandedGun, "Reload2HandedGun" },
		{WeaponType::AssaultRifle, "ReloadAssaultRifle" },
		{WeaponType::ShotGun, "ReloadShotgun" },
		{WeaponType::MissileLauncher, "ReloadMissileLauncher"  },
		{WeaponType::Sniper, "ReloadAssaultRifle" },
		{WeaponType::MineLauncher, "ReloadOverheatBeamGun" },
	};

	static const std::unordered_map<WeaponType, std::string> kDefaultTransforms{
		{WeaponType::HandedGun, "RefPos2HandedGun_Action" },
		{WeaponType::AssaultRifle, "RefPosAssaultRifle_Action" },
		{WeaponType::ShotGun, "RefPosShotgun_Action" },
		{WeaponType::MissileLauncher, "RefPosMissileLauncher_Action" },
		{WeaponType::Sniper, "RefPosSniper_Action" },
		{WeaponType::MineLauncher, "RefPosLightningGun_Action" },
	};

	base::Awake();
	mController = mObject->GetObj<GameObject>()->GetAnimator()->GetController();
	mSpine = mObject->FindFrame("Humanoid_ Spine1");

	const std::function<void()>& endReloadCallback = std::bind(&Script_NetworkRemotePlayer::EndReloadCallback, this);
	const std::function<void()>& startReloadCallback = std::bind(&Script_NetworkRemotePlayer::StartReloadCallback, this);
	for (int i = 0; i < static_cast<int>(WeaponName::_count); ++i) {
		WeaponName weaponName = static_cast<WeaponName>(i);
		const std::string weaponModelName = Script_Weapon::GetWeaponModelName(weaponName);

		if (weaponModelName == "") {
			continue;
		}

		auto& weapon = mWeapons[weaponName] = BattleScene::I->Instantiate(weaponModelName, ObjectTag::Untagged, false);

		WeaponType weaponType = gkWeaponTypeMap.at(weaponName);
		Transform* transform = mObject->FindFrame(kDefaultTransforms.at(weaponType));
		transform->SetChild(weapon->GetShared());

		const auto& realodMotion = mReloadMotions[static_cast<int>(weaponName)] = mController->FindMotionByName(kReloadMotions.at(weaponType), "Body");
		realodMotion->AddCallback(endReloadCallback, realodMotion->GetMaxFrameRate() - 14);
		realodMotion->AddStartCallback(startReloadCallback);
	}
	mCurrWeaponName = WeaponName::H_Lock;
	mCrntWeapon = mWeapons[mCurrWeaponName];
	mCrntWeapon->SetActive(true);

	for (const auto& [weaponName, weapon] : mWeapons) {
		switch (weaponName) {
		case WeaponName::H_Lock:
			mWeaponScripts[weapon] = weapon->AddComponent<Script_Weapon_Pistol>();
			break;
		case WeaponName::DBMS:
			mWeaponScripts[weapon] = weapon->AddComponent<Script_Weapon_DBMS>();
			break;
		case WeaponName::SkyLine:
			mWeaponScripts[weapon] = weapon->AddComponent<Script_Weapon_Skyline>();
			break;
		case WeaponName::Burnout:
			mWeaponScripts[weapon] = weapon->AddComponent<Script_Weapon_Burnout>();
			break;
		case WeaponName::PipeLine:
			mWeaponScripts[weapon] = weapon->AddComponent<Script_Weapon_PipeLine>();
			ResetBoltActionMotionSpeed(weapon->GetComponent<Script_Weapon>());
			break;
		case WeaponName::MineLauncher:
			mWeaponScripts[weapon] = weapon->AddComponent<Script_Weapon_MineLauncher>();
			break;
		default:
			assert(0);
			break;
		}

		mWeaponScripts[weapon]->Awake();
	}

	mAirstrike = std::make_shared<Airstrike>();
	mAirstrike->Init();

	mRemoteAbilityShield = mObject->AddComponent<Script_Remote_Ability_Shield>(true, false);
	mRemoteAbilityCloaking = mObject->AddComponent<Script_Remote_Ability_Cloaking>(true, false);
	mRemoteAbilityMindControl = mObject->AddComponent<Script_Remote_Ability_MindControl>(true, false);
	
	mBattleUI = BattleScene::I->GetManager()->GetComponent<Script_BattleManager>()->GetUI();
	mBattleUI->CreatePlayerUI(this);
	mBattleUI->SetWeapon(mObject, mWeaponScripts[mCrntWeapon]);
}

void Script_NetworkRemotePlayer::Update()
{
	base::Update();

	if (mController) {
		mController->SetValueOnly("Vertical", fabs(mParamV) > 0.1f ? mParamV : 0.f);
		mController->SetValueOnly("Horizontal", fabs(mParamH) > 0.1f ? mParamH : 0.f);
	}
	if (mFireCnt > 0) {
		--mFireCnt;
		mWeaponScripts.at(mCrntWeapon)->FireBullet_Force();
		mBattleUI->UpdateWeapon(mObject);
	}
}

void Script_NetworkRemotePlayer::OnDestroy()
{
	base::OnDestroy();

	for (auto& [weaponName, weapon] : mWeapons) {
		weapon->mParent = nullptr;
		BattleScene::I->RemoveDynamicObject(weapon);
	}

	mBattleUI->RemovePlayer(mObject);
}

void Script_NetworkRemotePlayer::LateUpdate()
{
	base::LateUpdate();

	/* CurrPos ---- PktPos ----------- TargetPos */

	Vec3& curpos = mObject->GetPosition();
	Vec3& TarPos = mCurrExtraPolated_Data.TargetPos;
#define LERP_DR
#ifdef LERP_DR
	/* MOVE */

	Vec3 prevPos = mObject->GetPosition();

	Vec3 newPosition = lerp(curpos, TarPos, DeltaTime(), mCurrExtraPolated_Data.Velocity);
	mObject->SetPosition(newPosition);

	if (!mIsAim) {
		RotateTo(mObject, mCurrExtraPolated_Data.MoveDir, mRotationSpeed);
	}
	else {
		if (fabs(mSpineAngle) > FLT_EPSILON) {
			float crntAngle = Vector3::SignedAngle(Vector3::Forward, mSpine->GetUp().xz(), Vector3::Up);
			mSpine->RotateGlobal(Vector3::Up, mSpineAngle - crntAngle);
		}
	}

	Vec3 crntPos = mObject->GetPosition();

	if ((prevPos - crntPos).Length() > 0.01f) {
		UpdateParam(1, mParamV);
	}

	return;
#endif

#define LERP_DR
#ifdef LERP_DR

#endif

//#define BEZIER_CURVE_DR
#ifdef BEZIER_CURVE_DR

		/* º£Áö¾î °î¼±¿¡ µû¶ó¼­ º¸°£ÇÏ¸ç ¿òÁ÷ÀÎ´Ù. */
	/// +------------------------------------------
	///		BEZIER CURVE with Dead Reckoning 
	/// ------------------------------------------+

	mBezierTime += DeltaTime();

	if (mBezierTime >= 1.f)
		mBezierTime = 1.f;

	Vec3 point = Bezier_Curve_3(curpos, TarPos, mBezierTime);
	mObject->SetPosition(point);

	return;
#endif 
}

void Script_NetworkRemotePlayer::UpdateData(const void* data)
{
}

void Script_NetworkRemotePlayer::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
}

void Script_NetworkRemotePlayer::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
}



// 3Â÷¿ø º£Áö¾î °î¼± »ý¼º 
Vec3 Script_NetworkRemotePlayer::Bezier_Curve_3(Vec3 currpos, Vec3 targetpos, float t)
{
	float acc = 0.f; // °¡¼Óµµ´Â ¾²Áö ¾Ê´Â´Ù. 
	ExtData data = mCurrExtraPolated_Data;

	// ½ÃÀÛÁ¡
	Vec3 p0 = currpos;

	// Á¾·áÁ¡
	Vec3 p3 = targetpos;

	// °¢ Á¦¾îÁ¡ÀÇ À§Ä¡ °è»ê
	Vec3 p1 = p0 + (p3 - p0) * (1.0f / 3.0f);
	Vec3 p2 = p0 + (p3 - p0) * (2.0f / 3.0f);

	// º£Áö¾î °î¼± °è»ê
	Vec3 point = Bezier_Curve(p0, p1, p2, p3, t);

	return point;
}


// º£Áö¾î °î¼±À» °è»êÇÏ´Â ÇÔ¼ö
Vec3 Script_NetworkRemotePlayer::Bezier_Curve(Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, float t)
{
	// º£Áö¾î °î¼±ÀÇ °ø½ÄÀ» »ç¿ëÇÏ¿© À§Ä¡¸¦ °è»ê
	float u   = 1.0f - t;
	float tt  = t * t;
	float uu  = u * u;
	float uuu = uu * u;
	float ttt = tt * t;

	Vec3 point = (uuu * p0) + (3.0f * uu * t * p1) + (3.0f * u * tt * p2) + (ttt * p3);

	return point;
}

float Script_NetworkRemotePlayer::GetYAngle()
{
	return mSpine->GetYAngle();
}


float Script_NetworkRemotePlayer::Distance(const Vec3& v1, const Vec3& v2)
{
	return sqrt((v1.x - v2.x) * (v1.x - v2.x) +
		(v1.y - v2.y) * (v1.y - v2.y) +
		(v1.z - v2.z) * (v1.z - v2.z));
}

void Script_NetworkRemotePlayer::SetCurrWeaponName(FBProtocol::ITEM_TYPE weaponType)
{
	if (mCrntWeapon) {
		mCrntWeapon->SetActive(false);
	}

	switch (weaponType)
	{
	case FBProtocol::ITEM_TYPE_WEAPON_H_LOOK:
		mCurrWeaponName = WeaponName::H_Lock;
		break;
	case FBProtocol::ITEM_TYPE_WEAPON_DBMS:
		mCurrWeaponName = WeaponName::DBMS;
		break;
	case FBProtocol::ITEM_TYPE_WEAPON_PIPELINE:
		mCurrWeaponName = WeaponName::PipeLine;
		break;
	case FBProtocol::ITEM_TYPE_WEAPON_BURNOUT:
		mCurrWeaponName = WeaponName::Burnout;
		break;
	case FBProtocol::ITEM_TYPE_WEAPON_SKYLINE:
		mCurrWeaponName = WeaponName::SkyLine;
		break;
	case FBProtocol::ITEM_TYPE_WEAPON_AIR_STRIKE:
		mCurrWeaponName = WeaponName::Airstrike;
		break;
	default:
		mCrntWeapon = nullptr;
		return;
	}

	if (mWeapons.count(mCurrWeaponName)) {
		mCrntWeapon = mWeapons[mCurrWeaponName];
		mCrntWeapon->SetActive(true);
		mBattleUI->SetWeapon(mObject, mWeaponScripts[mCrntWeapon]);
	}
}

void Script_NetworkRemotePlayer::EndReloadCallback()
{
	if (mWeaponScripts[mCrntWeapon]) {
		mWeaponScripts[mCrntWeapon]->EndReload();
	}

	mBattleUI->UpdateWeapon(mObject);
	mIsReloading = false;
}

void Script_NetworkRemotePlayer::StartReloadCallback()
{
	if (mIsReloading) {
		return;
	}
	mIsReloading = true;

	if (mWeaponScripts[mCrntWeapon]) {
		mWeaponScripts[mCrntWeapon]->CheckReload();

		auto motion = mReloadMotions[static_cast<int>(mCurrWeaponName)];
		SetMotionSpeed(motion, mWeaponScripts[mCrntWeapon]->GetReloadTime());
	}

	mBattleUI->UpdateWeapon(mObject);
}

void Script_NetworkRemotePlayer::ToggleAbilityShield()
{
	mRemoteAbilityShield->Toggle();
}

void Script_NetworkRemotePlayer::ToggleAbilityCloaking()
{
	mRemoteAbilityCloaking->Toggle();
}

void Script_NetworkRemotePlayer::ToggleAbilityMindControl(Object* pickedTarget)
{
	// ¸¶ÀÎµå ÄÁÆ®·Ñ »óÅÂ¶ó¸é 
	if (pickedTarget->mObjectCB.MindRimFactor > 0.f) {
		pickedTarget->mObjectCB.MindRimFactor = 0.f;
	}
	else {
		pickedTarget->mObjectCB.MindRimFactor = 1.f;
	}
}

Vec3 Script_NetworkRemotePlayer::GetDirection(Vec3 dir)
{
	float length = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	return { dir.x / length, dir.y / length, dir.z / length };
}

Vec3 Script_NetworkRemotePlayer::CalculateDirection(float yAngleRadian)
{
	// x ¹× z ¹æÇâ º¤ÅÍ °è»ê
	float xDir = std::sin(yAngleRadian);
	float zDir = std::cos(yAngleRadian);

	Vec3 dir = Vec3(xDir, 0.0f, zDir); // y ¹æÇâÀº °í·ÁÇÏÁö ¾ÊÀ½
	dir.Normalize();
	return dir;
}

Vec3 Script_NetworkRemotePlayer::lerp(Vec3 CurrPos, Vec3 TargetPos, float PosLerpParam)
{
	//return CurrPos + t * (TargetPos - CurrPos);

	return Vec3((1.0f - PosLerpParam) * CurrPos.x + PosLerpParam * TargetPos.x,
		(1.0f - PosLerpParam) * CurrPos.y + PosLerpParam * TargetPos.y,
		(1.0f - PosLerpParam) * CurrPos.z + PosLerpParam * TargetPos.z);
}

Vec3 Script_NetworkRemotePlayer::lerp(Vec3 CurrPos, Vec3 TargetPos, float Deltatime, float speed)
{
	float distanceToMove = speed * Deltatime;
	float totalDistance = (TargetPos - CurrPos).Length();

	float PosLerpParam = 0.f;
	if (totalDistance > 0.f) {
		PosLerpParam = distanceToMove / totalDistance;
	}

	// clamp
	if (PosLerpParam > 1.f)
		PosLerpParam = 1.f;



	return Vec3((1.0f - PosLerpParam) * CurrPos.x + PosLerpParam * TargetPos.x,
				(1.0f - PosLerpParam) * CurrPos.y + PosLerpParam * TargetPos.y,
				(1.0f - PosLerpParam) * CurrPos.z + PosLerpParam * TargetPos.z);
}

Vec3 Script_NetworkRemotePlayer::quadraticInterpolation(const Vec3& p0, const Vec3& p1, const Vec3& p2, float t)
{
	float oneMinusT = 1.0f - t;
	Vec3 result = p0 * (oneMinusT * oneMinusT) + p1 * (2.0f * oneMinusT * t) + p2 * (t * t);
	return result;
}

float Script_NetworkRemotePlayer::GetYAngleFromQuaternion(const Vec4& rotationQuaternion)
{
	// QuaternionÀÇ x, y, z, w °ªÀ» ÃßÃâÇÕ´Ï´Ù.
	float x = rotationQuaternion.x;
	float y = rotationQuaternion.y;
	float z = rotationQuaternion.z;
	float w = rotationQuaternion.w;

	// y °¢µµ¸¦ ±¸ÇÏ´Â °ø½ÄÀ» Àû¿ëÇÕ´Ï´Ù.
	float yAngle = asin(-2.0f * (x * z - w * y)) * (180.0f / XM_PI);

	return yAngle;
}

void Script_NetworkRemotePlayer::RotateTo(float yAngle, float ySpineAngle)
{
	if (yAngle < -1000) {
		mIsAim = false;
		mSpineAngle = 0.f;
		return;
	}
	mIsAim = true;

	Vec3 dir = Vector3::Rotate(Vector3::Forward, 0, yAngle, 0);
	RotateTo(mObject, dir, 900);
	mSpineAngle = ySpineAngle;
}

void Script_NetworkRemotePlayer::RotateTo(Transform* transform, const Vec3& dir, float speed)
{
	const float angle = Vector3::SignedAngle(transform->GetLook().xz(), dir, Vector3::Up);
	constexpr float smoothAngleBound = 10.f;
	// smooth rotation if angle over [smoothAngleBound] degree
	if (fabs(angle) > smoothAngleBound) {
		transform->Rotate(0, Math::Sign(angle) * speed * DeltaTime(), 0);
	}
	else if (fabs(angle) > FLT_EPSILON) {
		transform->Rotate(0, angle, 0);
	}
}


void Script_NetworkRemotePlayer::MoveToTargetPos(Vec3 TargetPos)
{
	Vec3 CurrPos = mObject->GetLocalPosition();
	mCurrMoveDir = TargetPos - CurrPos;
	mCurrMoveDir.Normalize();

	/* ÀÌµ¿ °Å¸® °è»ê */
	Vec3 movelen = Vec3();
	movelen.x = mCurrMoveDir.x * mCurrExtraPolated_Data.Velocity * DeltaTime();
	movelen.z = mCurrMoveDir.z * mCurrExtraPolated_Data.Velocity * DeltaTime();

	/* ÀÌµ¿ °Å¸® Á¶Á¤ */
	Vec3 RestMoveLen = TargetPos - CurrPos;


	mObject->Translate(Vec3(movelen.x, 0, movelen.z));
}

void Script_NetworkRemotePlayer::MoveToPacketPos(Vec3 PacketPos)
{
	Vec3 CurrPos = mObject->GetLocalPosition();
	Vec3 MoveDir = PacketPos - CurrPos;
	MoveDir.Normalize();

	if (mCurrExtraPolated_Data.MoveDir.x != MoveDir.x &&
		mCurrExtraPolated_Data.MoveDir.z != MoveDir.z) {

		MoveToTargetPos(PacketPos);
	}

}

void Script_NetworkRemotePlayer::SetExtrapolatedData(ExtData& extData)
{
	mBezierTime = 0.f;
	mPrevExtrapolated_Data = mCurrExtraPolated_Data;
	mCurrExtraPolated_Data = extData;
}

void Script_NetworkRemotePlayer::UpdateParams()
{
}

void Script_NetworkRemotePlayer::UpdateParam(float val, float& param)
{
	constexpr float kParamSpeed = 6.f;		// ï¿½Ä¶ï¿½ï¿½ï¿½ï¿? ï¿½ï¿½È¯ ï¿½Óµï¿½
	constexpr float kOppositeExtraSpeed = 8.f;		// ï¿½Ý´ï¿½ï¿½ï¿½ ï¿½Ìµï¿½ ï¿½ï¿½ ï¿½ß°ï¿½ ï¿½Ìµï¿½ ï¿½ï¿½È¯ ï¿½Óµï¿½

	int sign = Math::Sign(val);						// sign : ï¿½Ä¶ï¿½ï¿½ï¿½ï¿? ï¿½Ìµï¿½ ï¿½ï¿½ï¿½ï¿½ = ï¿½ï¿½ï¿½ï¿½ ï¿½Ô·Â°ï¿½ï¿½ï¿½ ï¿½ï¿½È£
	if (Math::IsZero(val)) {						//		  ï¿½Ô·ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½Ù¸ï¿½ ï¿½ï¿½ï¿½ï¿½ ï¿½Ä¶ï¿½ï¿½ï¿½ï¿½ï¿½ï¿? ï¿½Ý´ï¿½ ï¿½ï¿½È£
		if (Math::IsZero(param)) {
			return;
		}
		sign = -Math::Sign(param);
	}
	float before = param;
	param += (kParamSpeed * sign) * DeltaTime();	// ï¿½Ä¶ï¿½ï¿½ï¿½Í°ï¿? ï¿½ï¿½ï¿½ï¿½

	if (!Math::IsZero(val)) {
		if (fabs(param) < 0.5f && (fabs(before) > fabs(param))) {	// ï¿½Ý´ï¿½ï¿½ï¿½ ï¿½Ìµï¿½ ï¿½ï¿½
			param += (sign * kOppositeExtraSpeed) * DeltaTime();	// ï¿½ß°ï¿½ ï¿½ï¿½È¯ ï¿½Óµï¿½ ï¿½ï¿½ï¿½ï¿½
		}
		else if (fabs(param) >= fabs(before)) {						// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½Ìµï¿½ ï¿½ï¿½
			param = std::clamp(param, -fabs(val), fabs(val));		// paramï¿½ï¿½ valï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½Ïµï¿½ï¿½ï¿½ ï¿½Ñ´ï¿½.

			// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ && 0ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿? ï¿½ï¿½ï¿? ï¿½Ø´ï¿½ ï¿½Ä¶ï¿½ï¿½ï¿½Í´ï¿? ï¿½ï¿½ï¿½ï¿½ï¿½Ñ´ï¿½.
			if (fabs(fabs(param) - fabs(before)) < 0.001f && fabs(param) < 0.1f) {								// 0ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½Ï¸ï¿½ 0ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½
				param = 0.f;
			}
		}
	}

	param = std::clamp(param, -1.f, 1.f);		// -1 ~ 1 ï¿½ï¿½ï¿½Ì·ï¿½ ï¿½ï¿½ï¿½ï¿½
}

void Script_NetworkRemotePlayer::FireBullet(const Vec3& firePos, const Vec3& look)
{
	if (!mCrntWeapon) {
		return;
	}

	if (mCurrWeaponName == WeaponName::Airstrike) {
		if (mAirstrike) {
			mAirstrike->Fire(firePos, look);
		}
	}
	else {
		++mFireCnt;
	}
}

void Script_NetworkRemotePlayer::ResetBoltActionMotionSpeed(rsptr<Script_Weapon> weapon)
{
	if (weapon->GetWeaponType() != WeaponType::Sniper) {
		return;
	}

	auto boltActionMotion = mController->FindMotionByName("BoltActionSniper", "Body");

	// motion speed
	constexpr float decTime = 0.1f;
	const float fireDelay = weapon->GetFireDelay();
	SetMotionSpeed(boltActionMotion, fireDelay - decTime);
}

void Script_NetworkRemotePlayer::SetMotionSpeed(AnimatorMotion* motion, float time)
{
	if (time <= 0.f) {
		return;
	}

	const float motionSpeed = motion->GetMaxLength() / time;
	motion->ResetOriginSpeed(motionSpeed);
}