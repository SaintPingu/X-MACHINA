#include "stdafx.h"
#include "Script_RemotePlayer.h"
#include "Timer.h"
#include "ClientNetwork/Include/LogManager.h"
void Script_RemotePlayer::Awake()
{
	base::Awake();

}

void Script_RemotePlayer::LateUpdate()
{
	base::LateUpdate();

	t += DeltaTime() ;
	
	Vec3 CurrPos   = mObject->GetLocalPosition();
	Vec3 TargetPos = mExtraPolated_Data.TargetPos; //mCurrPacketPos + (mCurrPacketPos - mPrevPacketPos);
	Vec3 MoveDir   = (mCurrPacketPos - mPrevPacketPos);
	Vec3 MoveLen   = MoveDir;
	MoveDir.Normalize();

	Vec3 NewPos = lerp(CurrPos, TargetPos, t);
	mObject->SetPosition(NewPos);

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
	movelen.x = mCurrMoveDir.x * mExtraPolated_Data.Velocity * DeltaTime();
	movelen.z = mCurrMoveDir.z * mExtraPolated_Data.Velocity * DeltaTime();

	/* 이동 거리 조정 */
	Vec3 RestMoveLen = TargetPos - CurrPos;


	mObject->Translate(Vec3(movelen.x, 0, movelen.z));
}

void Script_RemotePlayer::MoveToPacketPos(Vec3 PacketPos)
{
	Vec3 CurrPos = mObject->GetLocalPosition();
	Vec3 MoveDir = PacketPos - CurrPos;
	MoveDir.Normalize();

	if (mExtraPolated_Data.MoveDir.x != MoveDir.x && 
		mExtraPolated_Data.MoveDir.z != MoveDir.z) {

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