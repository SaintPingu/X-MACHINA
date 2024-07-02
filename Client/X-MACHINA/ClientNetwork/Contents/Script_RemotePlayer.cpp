#include "stdafx.h"
#include "Script_RemotePlayer.h"

#include "Timer.h"
#include "Object.h"
#include "Animator.h"
#include "AnimatorController.h"
#include "ClientNetwork/Include/LogManager.h"

#include "Script_GroundObject.h"
void Script_RemotePlayer::Awake()
{
	base::Awake();

}

void Script_RemotePlayer::LateUpdate()
{
	base::LateUpdate();

	/* CurrPos ---- PktPos ----------- TargetPos */


	Vec3& curpos = mObject->GetPosition();
	Vec3& TarPos = mCurrExtraPolated_Data.TargetPos;

//#define BEZIER_CURVE_DR
#ifdef BEZIER_CURVE_DR

		/* ������ ��� ���� �����ϸ� �����δ�. */
	/// +------------------------------------------
	///		BEZIER CURVE with Dead Reckoning 
	/// ------------------------------------------+

	mBezierTime += DeltaTime();//  *BEZIER_WEIGHT_ADJUSTMENT;

	if (mBezierTime >= 1.f)
		mBezierTime = 1.f;

	Vec3 point = Bezier_Curve_3(curpos, TarPos, mBezierTime);
	mObject->SetPosition(point);

	return;
#else

	switch (mCurrExtraPolated_Data.MoveState)
	{
	case ExtData::MOVESTATE::Start:
	case ExtData::MOVESTATE::Progress:
	{
		if (mCurrExtraPolated_Data.MoveState == ExtData::MOVESTATE::Start) {
			//std::cout << "START \n";
			//std::cout << "CURPOS : " << curpos.x << " " << curpos.z << "  TARPOS : " << TarPos.x << " " << TarPos.z << "\n";
		}
		mCurrMoveDir = TarPos - curpos; mCurrMoveDir.Normalize();
		curpos += mCurrMoveDir * mCurrExtraPolated_Data.Velocity * DeltaTime();
		mObject->SetPosition(curpos);


	}
		break;
	case ExtData::MOVESTATE::End:
	{
		mBezierTime += DeltaTime();//  *BEZIER_WEIGHT_ADJUSTMENT;

		if (mBezierTime >= 1.f)
			mBezierTime = 1.f;

		//Vec3 point = Bezier_Curve_3(curpos, TarPos, mBezierTime);
		Vec3 point = lerp(curpos, TarPos, mBezierTime);
		mObject->SetPosition(point);
	}
		break;
	}
	mPrevMoveDir = mCurrMoveDir;

	return;
#endif

}

void Script_RemotePlayer::UpdateData(const void* data)
{
}



// 3���� ������ � ���� 
Vec3 Script_RemotePlayer::Bezier_Curve_3(Vec3 currpos, Vec3 targetpos, float t)
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
Vec3 Script_RemotePlayer::Bezier_Curve(Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, float t)
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

Vec3 Script_RemotePlayer::GetDirection(Vec3 dir)
{
	float length = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	return { dir.x / length, dir.y / length, dir.z / length };
}

Vec3 Script_RemotePlayer::CalculateDirection(float yAngleRadian)
{
	// x �� z ���� ���� ���
	float xDir = std::sin(yAngleRadian);
	float zDir = std::cos(yAngleRadian);

	Vec3 dir = Vec3(xDir, 0.0f, zDir); // y ������ ������� ����
	dir.Normalize();
	return dir;
}

Vec3 Script_RemotePlayer::lerp(Vec3 CurrPos, Vec3 TargetPos, float PosLerpParam)
{
	//return CurrPos + t * (TargetPos - CurrPos);

	return Vec3((1.0f - PosLerpParam) * CurrPos.x + PosLerpParam * TargetPos.x,
		(1.0f - PosLerpParam) * CurrPos.y + PosLerpParam * TargetPos.y,
		(1.0f - PosLerpParam) * CurrPos.z + PosLerpParam * TargetPos.z);
}

Vec3 Script_RemotePlayer::quadraticInterpolation(const Vec3& p0, const Vec3& p1, const Vec3& p2, float t)
{
	float oneMinusT = 1.0f - t;
	Vec3 result = p0 * (oneMinusT * oneMinusT) + p1 * (2.0f * oneMinusT * t) + p2 * (t * t);
	return result;
}

float Script_RemotePlayer::GetYAngleFromQuaternion(const Vec4& rotationQuaternion)
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

void Script_RemotePlayer::RotateTo(const Vec3& Angle)
{
	// ������ ��ȿ���� Ȯ���ϰ� ��ȿ���� ������ ����
	if (Angle.LengthSquared() < FLT_EPSILON) {
		return;
	}

	// ��ü�� ���� ���� ����
	Vec3 currentDir = mObject->GetLook().xz();
	currentDir.Normalize();

	// ��ǥ ���� ����
	Vec3 targetDir = Vector3::Forward; // �⺻������ ������ ���ϵ��� ����

	// ������ ���� ���ͷ� ��ȯ
	targetDir = Vector3::Rotate(targetDir, Angle.y);

	// ��ü�� ȸ����Ű�� ���� ���
	float rotationAngle = Vector3::SignedAngle(currentDir, targetDir, Vector3::Up);

	// ȸ�� ������ ���� �ε巯�� ȸ�� �Ǵ� ��� ȸ��
	constexpr float smoothAngleBound = 10.f; // �ε巯�� ȸ���� ���� �Ӱ谪
	if (fabs(rotationAngle) > smoothAngleBound) {
		// �ε巯�� ȸ��
		mObject->Rotate(0, Math::Sign(rotationAngle) * mRotationSpeed * DeltaTime(), 0);
	}
	else if (fabs(rotationAngle) > FLT_EPSILON) {
		// ��� ȸ��
		mObject->Rotate(0, rotationAngle, 0);
	}
}


void Script_RemotePlayer::MoveToTargetPos(Vec3 TargetPos)
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

void Script_RemotePlayer::MoveToPacketPos(Vec3 PacketPos)
{
	Vec3 CurrPos = mObject->GetLocalPosition();
	Vec3 MoveDir = PacketPos - CurrPos;
	MoveDir.Normalize();

	if (mCurrExtraPolated_Data.MoveDir.x != MoveDir.x &&
		mCurrExtraPolated_Data.MoveDir.z != MoveDir.z) {

		MoveToTargetPos(PacketPos);
	}

}

void Script_RemotePlayer::SetExtrapolatedData(ExtData& extData)
{
	mBezierTime = 0.f;
	mPrevExtrapolated_Data = mCurrExtraPolated_Data;
	mCurrExtraPolated_Data = extData;
}
