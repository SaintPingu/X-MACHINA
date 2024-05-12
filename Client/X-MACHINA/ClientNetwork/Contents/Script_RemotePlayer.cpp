#include "stdafx.h"
#include "Script_RemotePlayer.h"
#include "Timer.h"
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

	mMoveState = mCurrExtraPolated_Data.MoveState;

	switch (mMoveState)
	{
	case ExtData::MOVESTATE::Start:
	{
		mMoveState = ExtData::MOVESTATE::Progress;
		mObject->SetPosition(mCurrPacketPos);
		Vec3 Rot = mCurrExtraPolated_Data.TargetRot;
		mObject->ResetRotation(Rot.y);

	}
	break;
	case ExtData::MOVESTATE::Progress:
	{
		Vec3 MoveVal = mCurrExtraPolated_Data.MoveDir * mCurrExtraPolated_Data.Velocity * DeltaTime();
		mObject->Translate(MoveVal);
		Vec3 Rot = mCurrExtraPolated_Data.TargetRot;
		mObject->ResetRotation(Rot.y);


	}
	break;
	case ExtData::MOVESTATE::End:
	{
		mObject->SetPosition(mCurrPacketPos);
		Vec3 Rot = mCurrExtraPolated_Data.TargetRot;
		mObject->ResetRotation(Rot.y);

	}
	break;
	}

	return;

	t += DeltaTime() ;

	
	Vec3 CurrPos = mObject->GetLocalPosition();
	Vec3 PacketPos = mCurrPacketPos;
	Vec3 TargetPos = mCurrExtraPolated_Data.TargetPos;

	//Vec3 MoveDir   = mExtraPolated_Data.MoveDir;
	Vec3 MoveDir = mCurrPacketPos - mPrevPacketPos;

	//Vec3 MoveDir   = (mCurrPacketPos - mPrevPacketPos);
	Vec3 MoveLen = MoveDir;
	MoveDir.Normalize();

	Vec3 Move = MoveDir * mCurrExtraPolated_Data.Velocity * DeltaTime();
	mObject->Translate(Move);



	//Vec3 NewPos = lerp(CurrPos, TargetPos, t);
	//mObject->SetPosition(NewPos);

	/*Vec3 movelen = Vec3();
	movelen.x    = MoveDir.x * mExtraPolated_Data.Velocity * DeltaTime() * MoveLen.x;
	movelen.z    = MoveDir.z * mExtraPolated_Data.Velocity * DeltaTime() * MoveLen.z;

	mObject->Translate(Vec3(movelen.x, 0, movelen.z));*/


	if (t >= 1.f)
		t = 0.f;

	/* CurrPos ---- PktPos ----------- TargetPos */
	//CurrPos = lerp(PktPos, TargetPos, DeltaTime());
	//mObject->SetPosition(CurrPos);

	//MoveToPacketPos(mCurrPacketPos);
	//MoveToTargetPos(mExtraPolated_Data.Pos);

	
	/* CurrPos ---- Move Auto ----> TargetPos */

	//Vec3 CurrPos   = mObject->GetPosition();
	//Vec3 TargetPos = mExtraPolated_Data.Pos;
	//Vec3 MoveDir   = TargetPos - CurrPos;
	//MoveDir.Normalize();


	//Vec3 movelen = Vec3();
	//movelen.x = MoveDir.x * mExtraPolated_Data.Velocity * DeltaTime();
	//movelen.z = MoveDir.z * mExtraPolated_Data.Velocity * DeltaTime();

	//mObject->Translate(Vec3(movelen.x, 0, movelen.z));
	
	//std::cout << "	mObject->Translate(dirVec * mExtraPolated_Data.Velocity * DeltaTime())\n";


}

void Script_RemotePlayer::UpdateData(const void* data)
{
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