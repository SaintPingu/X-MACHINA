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

		/* 베지어 곡선에 따라서 보간하며 움직인다. */
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



// 3차원 베지어 곡선 생성 
Vec3 Script_RemotePlayer::Bezier_Curve_3(Vec3 currpos, Vec3 targetpos, float t)
{
	float acc = 0.f; // 가속도는 쓰지 않는다. 
	ExtData data = mCurrExtraPolated_Data;

	// 시작점
	Vec3 p0 = currpos;

	// 종료점
	Vec3 p3 = targetpos;

	// 각 제어점의 위치 계산
	Vec3 p1 = p0 + (p3 - p0) * (1.0f / 3.0f);
	Vec3 p2 = p0 + (p3 - p0) * (2.0f / 3.0f);

	// 베지어 곡선 계산
	Vec3 point = Bezier_Curve(p0, p1, p2, p3, t);

	return point;
}


// 베지어 곡선을 계산하는 함수
Vec3 Script_RemotePlayer::Bezier_Curve(Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, float t)
{
	// 베지어 곡선의 공식을 사용하여 위치를 계산
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
	// x 및 z 방향 벡터 계산
	float xDir = std::sin(yAngleRadian);
	float zDir = std::cos(yAngleRadian);

	Vec3 dir = Vec3(xDir, 0.0f, zDir); // y 방향은 고려하지 않음
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
	// Quaternion의 x, y, z, w 값을 추출합니다.
	float x = rotationQuaternion.x;
	float y = rotationQuaternion.y;
	float z = rotationQuaternion.z;
	float w = rotationQuaternion.w;

	// y 각도를 구하는 공식을 적용합니다.
	float yAngle = asin(-2.0f * (x * z - w * y)) * (180.0f / XM_PI);

	return yAngle;
}

void Script_RemotePlayer::RotateTo(const Vec3& Angle)
{
	// 각도가 유효한지 확인하고 유효하지 않으면 종료
	if (Angle.LengthSquared() < FLT_EPSILON) {
		return;
	}

	// 물체의 현재 방향 벡터
	Vec3 currentDir = mObject->GetLook().xz();
	currentDir.Normalize();

	// 목표 방향 벡터
	Vec3 targetDir = Vector3::Forward; // 기본적으로 전방을 향하도록 설정

	// 각도를 방향 벡터로 변환
	targetDir = Vector3::Rotate(targetDir, Angle.y);

	// 물체를 회전시키는 각도 계산
	float rotationAngle = Vector3::SignedAngle(currentDir, targetDir, Vector3::Up);

	// 회전 각도에 따라 부드러운 회전 또는 즉시 회전
	constexpr float smoothAngleBound = 10.f; // 부드러운 회전을 위한 임계값
	if (fabs(rotationAngle) > smoothAngleBound) {
		// 부드러운 회전
		mObject->Rotate(0, Math::Sign(rotationAngle) * mRotationSpeed * DeltaTime(), 0);
	}
	else if (fabs(rotationAngle) > FLT_EPSILON) {
		// 즉시 회전
		mObject->Rotate(0, rotationAngle, 0);
	}
}


void Script_RemotePlayer::MoveToTargetPos(Vec3 TargetPos)
{
	Vec3 CurrPos = mObject->GetLocalPosition();
	mCurrMoveDir = TargetPos - CurrPos;
	mCurrMoveDir.Normalize();

	/* 이동 거리 계산 */
	Vec3 movelen = Vec3();
	movelen.x = mCurrMoveDir.x * mCurrExtraPolated_Data.Velocity * DeltaTime();
	movelen.z = mCurrMoveDir.z * mCurrExtraPolated_Data.Velocity * DeltaTime();

	/* 이동 거리 조정 */
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
