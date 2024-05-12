#pragma once

#include "Script_Network.h"

/* Extrapolation Data - 패킷을 통한 위치 예측을 위해 필요한 데이터 */



struct ExtData
{
	enum class MOVESTATE {
		NONE, Start, Progress, End,
	};
	long long PingTime  = {}; // 다음 Transform 패킷이 오기까지 걸리는 시간 ( 추측 - ( SPkt_Transform Recv Interval + Remote Client Latency + My Latency )
	MOVESTATE MoveState = MOVESTATE::NONE;
	float	  Velocity  = {};
	Vec3	  TargetPos = {};
	Vec3	  TargetRot = {};
	Vec3	  MoveDir   = {};

	
};

class Script_RemotePlayer : public Script_Network 
{	
	COMPONENT(Script_RemotePlayer, Script_Network)

private:
	ExtData mCurrExtraPolated_Data = {};
	ExtData mPrevExtrapolated_Data = {};

	Vec3 mCurrMoveDir = Vec3();
	Vec3 mCurrPacketPos = Vec3();
	Vec3 mPrevPacketPos = Vec3();

	ExtData::MOVESTATE mMoveState;
	float mRotationSpeed = 360.f;

	float t = 0;
public:
	virtual void Awake() override;
	virtual void LateUpdate() override;
	virtual void UpdateData(const void* data) override;


public:
	void MoveToTargetPos(Vec3 TargetPos);
	void MoveToPacketPos(Vec3 PacketPos);


public:
	void SetExtrapolatedData(ExtData& extData)							{ mPrevExtrapolated_Data = mCurrExtraPolated_Data; mCurrExtraPolated_Data = extData; }
	void SetExtrapolated_Pos(Vec3 pos)									{ mCurrExtraPolated_Data.TargetPos      = pos;									 }	
	void SetExtrapolated_Rot(Vec3 rot)									{ mCurrExtraPolated_Data.TargetRot      = rot;									 }
	void SetExtrapolated_PingTime(long long pingTime)					{ mCurrExtraPolated_Data.PingTime       = pingTime;								 }
	void SetExtrapolated_MoveState(ExtData::MOVESTATE movestate)		{ mCurrExtraPolated_Data.MoveState      = movestate;							 }
	
	void SetPacketPos(Vec3 pos)											{ mPrevPacketPos                    = mCurrPacketPos;  mCurrPacketPos = pos; }

public:
	Vec3 GetDirection(Vec3 dir);
	Vec3 CalculateDirection(float yAngleRadian);
	Vec3 lerp(Vec3 CurrPos, Vec3 TargetPos, float PosLerpParam);
	Vec3 quadraticInterpolation(const Vec3& p0, const Vec3& p1, const Vec3& p2, float t);
	float GetYAngleFromQuaternion(const Vec4& rotationQuaternion);


public:

	void RotateTo(const Vec3& Angle);


};

