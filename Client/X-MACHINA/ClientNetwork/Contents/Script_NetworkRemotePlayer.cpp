#include "stdafx.h"
#include "Script_NetworkRemotePlayer.h"

#include "Script_Weapon.h"

#include "Timer.h"
#include "Object.h"
#include "Animator.h"
#include "AnimatorController.h"
#include "BattleScene.h"
#include "ClientNetwork/Include/LogManager.h"

void Script_NetworkRemotePlayer::Awake()
{
	static const std::unordered_map<WeaponType, std::string> kDefaultTransforms{
	{WeaponType::HandedGun, "RefPos2HandedGun_Action" },
	{WeaponType::AssaultRifle, "RefPosAssaultRifle_Action" },
	{WeaponType::ShotGun, "RefPosShotgun_Action" },
	{WeaponType::MissileLauncher, "RefPosMissileLauncher_Action" },
	{WeaponType::Sniper, "RefPosSniper_Action" },
	};

	base::Awake();
	mController = mObject->GetObj<GameObject>()->GetAnimator()->GetController();
	mSpine = mObject->FindFrame("Humanoid_ Spine1");

	for (int i = 0; i < static_cast<int>(WeaponName::_count); ++i) {
		WeaponName weaponName = static_cast<WeaponName>(i + 1);
		std::string weaponModelName = Script_Weapon::GetWeaponModelName(weaponName);

		auto& weapon = mWeapons[weaponName] = BattleScene::I->Instantiate(weaponModelName, ObjectTag::Unspecified, ObjectLayer::Default, false);

		WeaponType weaponType = gkWeaponTypeMap.at(weaponName);
		Transform* transform = mObject->FindFrame(kDefaultTransforms.at(weaponType));
		transform->SetChild(weapon->GetShared());
	}
	mWeapons[WeaponName::H_Lock]->SetActive(true);
}

void Script_NetworkRemotePlayer::Update()
{
	base::Update();

	if (mController) {
		mController->SetValueOnly("Vertical", fabs(mParamV) > 0.1f ? mParamV : 0.f);
		mController->SetValueOnly("Horizontal", fabs(mParamH) > 0.1f ? mParamH : 0.f);
	}
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
		RotateTo(mCurrExtraPolated_Data.MoveDir, mRotationSpeed);
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

		/* ������ ��� ���� �����ϸ� �����δ�. */
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



// 3���� ������ � ���� 
Vec3 Script_NetworkRemotePlayer::Bezier_Curve_3(Vec3 currpos, Vec3 targetpos, float t)
{
	float acc = 0.f; // ���ӵ��� ���� �ʴ´�. 
	ExtData data = mCurrExtraPolated_Data;

	// ������
	Vec3 p0 = currpos;

	// ������
	Vec3 p3 = targetpos;

	// �� �������� ��ġ ���
	Vec3 p1 = p0 + (p3 - p0) * (1.0f / 3.0f);
	Vec3 p2 = p0 + (p3 - p0) * (2.0f / 3.0f);

	// ������ � ���
	Vec3 point = Bezier_Curve(p0, p1, p2, p3, t);

	return point;
}


// ������ ��� ����ϴ� �Լ�
Vec3 Script_NetworkRemotePlayer::Bezier_Curve(Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, float t)
{
	// ������ ��� ������ ����Ͽ� ��ġ�� ���
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

void Script_NetworkRemotePlayer::SetCurrWeaponName(FBProtocol::WEAPON_TYPE weaponType)
{
	if (mCurrWeaponName != WeaponName::None) {
		mWeapons[mCurrWeaponName]->SetActive(false);
	}

	switch (weaponType)
	{
	case FBProtocol::WEAPON_TYPE_H_LOOK:
		mCurrWeaponName = WeaponName::H_Lock;
		break;
	case FBProtocol::WEAPON_TYPE_DBMS:
		mCurrWeaponName = WeaponName::DBMS;
		break;
	case FBProtocol::WEAPON_TYPE_PIPELINE:
		mCurrWeaponName = WeaponName::PipeLine;
		break;
	case FBProtocol::WEAPON_TYPE_BURNOUT:
		mCurrWeaponName = WeaponName::Burnout;
		break;
	case FBProtocol::WEAPON_TYPE_SKYLINE:
		mCurrWeaponName = WeaponName::SkyLine;
		break;
	default:
		mCurrWeaponName = WeaponName::None;
		return;
	}

	mWeapons[mCurrWeaponName]->SetActive(true);
}


Vec3 Script_NetworkRemotePlayer::GetDirection(Vec3 dir)
{
	float length = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	return { dir.x / length, dir.y / length, dir.z / length };
}

Vec3 Script_NetworkRemotePlayer::CalculateDirection(float yAngleRadian)
{
	// x �� z ���� ���� ���
	float xDir = std::sin(yAngleRadian);
	float zDir = std::cos(yAngleRadian);

	Vec3 dir = Vec3(xDir, 0.0f, zDir); // y ������ ������� ����
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
	// Quaternion�� x, y, z, w ���� �����մϴ�.
	float x = rotationQuaternion.x;
	float y = rotationQuaternion.y;
	float z = rotationQuaternion.z;
	float w = rotationQuaternion.w;

	// y ������ ���ϴ� ������ �����մϴ�.
	float yAngle = asin(-2.0f * (x * z - w * y)) * (180.0f / XM_PI);

	return yAngle;
}

void Script_NetworkRemotePlayer::RotateTo(float yAngle)
{
	if (yAngle < -10000) {
		mIsAim = false;
		return;
	}
	mIsAim = true;

	Vec3 dir = Vector3::Rotate(Vector3::Forward, 0, yAngle, 0);
	RotateTo(dir, 600);
}

void Script_NetworkRemotePlayer::RotateTo(const Vec3& dir, float speed)
{
	const float angle = Vector3::SignedAngle(mObject->GetLook().xz(), dir, Vector3::Up);
	constexpr float smoothAngleBound = 10.f;
	// smooth rotation if angle over [smoothAngleBound] degree
	if (fabs(angle) > smoothAngleBound) {
		mObject->Rotate(0, Math::Sign(angle) * speed * DeltaTime(), 0);
	}
	else if (fabs(angle) > FLT_EPSILON) {
		mObject->Rotate(0, angle, 0);
	}
}


void Script_NetworkRemotePlayer::MoveToTargetPos(Vec3 TargetPos)
{
	Vec3 CurrPos = mObject->GetLocalPosition();
	mCurrMoveDir = TargetPos - CurrPos;
	mCurrMoveDir.Normalize();

	/* �̵� �Ÿ� ��� */
	Vec3 movelen = Vec3();
	movelen.x = mCurrMoveDir.x * mCurrExtraPolated_Data.Velocity * DeltaTime();
	movelen.z = mCurrMoveDir.z * mCurrExtraPolated_Data.Velocity * DeltaTime();

	/* �̵� �Ÿ� ���� */
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
	constexpr float kParamSpeed = 6.f;		// �Ķ����? ��ȯ �ӵ�
	constexpr float kOppositeExtraSpeed = 8.f;		// �ݴ��� �̵� �� �߰� �̵� ��ȯ �ӵ�

	int sign = Math::Sign(val);						// sign : �Ķ����? �̵� ���� = ���� �Է°��� ��ȣ
	if (Math::IsZero(val)) {						//		  �Է��� ���ٸ� ���� �Ķ������? �ݴ� ��ȣ
		if (Math::IsZero(param)) {
			return;
		}
		sign = -Math::Sign(param);
	}
	float before = param;
	param += (kParamSpeed * sign) * DeltaTime();	// �Ķ���Ͱ�? ����

	if (!Math::IsZero(val)) {
		if (fabs(param) < 0.5f && (fabs(before) > fabs(param))) {	// �ݴ��� �̵� ��
			param += (sign * kOppositeExtraSpeed) * DeltaTime();	// �߰� ��ȯ �ӵ� ����
		}
		else if (fabs(param) >= fabs(before)) {						// ������ �̵� ��
			param = std::clamp(param, -fabs(val), fabs(val));		// param�� val�� ���� ���ϵ��� �Ѵ�.

			// �������� ���� && 0�� �����? ���? �ش� �Ķ���ʹ�? �����Ѵ�.
			if (fabs(fabs(param) - fabs(before)) < 0.001f && fabs(param) < 0.1f) {								// 0�� �����ϸ� 0���� ����
				param = 0.f;
			}
		}
	}

	param = std::clamp(param, -1.f, 1.f);		// -1 ~ 1 ���̷� ����
}

void Script_NetworkRemotePlayer::FireBullet(const Vec3& ray)
{
	
}