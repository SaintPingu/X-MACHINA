#pragma once

#include "Script_Network.h"

/* Extrapolation Data - ��Ŷ�� ���� ��ġ ������ ���� �ʿ��� ������ */

constexpr float BEZIER_WEIGHT_ADJUSTMENT = 3.f; // ������ ����ġ ���� 

struct AnimMoveData
{
	float AnimParam_h = {};
	float AnimParam_v = {};
};


struct ExtData
{
	enum class MOVESTATE {
		NONE, Start, Progress, End,
	};	
	long long		PingTime     = {}; // ���� Transform ��Ŷ�� ������� �ɸ��� �ð� ( ���� - ( SPkt_Transform Recv Interval + Remote Client Latency + My Latency )
	MOVESTATE	MoveState = MOVESTATE::NONE;
	float		Velocity  = {};
	Vec3		TargetPos = {};
	Vec3		TargetRot = {};
	Vec3		MoveDir   = {};

	AnimMoveData Animdata = {};

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

	float mLerpVal = 0.f;

	double mBezierTime = 0.f;


	float t = 0;
public:
	virtual void Awake() override;
	virtual void LateUpdate() override;
	virtual void UpdateData(const void* data) override;


public:
	void MoveToTargetPos(Vec3 TargetPos);
	void MoveToPacketPos(Vec3 PacketPos);


public:
	void SetExtrapolatedData(ExtData& extData);


	void SetExtrapolated_Pos(Vec3 pos)									{ mCurrExtraPolated_Data.TargetPos      = pos;									 }	
	void SetExtrapolated_Rot(Vec3 rot)									{ mCurrExtraPolated_Data.TargetRot      = rot;									 }
	void SetExtrapolated_PingTime(double pingTime)					{ mCurrExtraPolated_Data.PingTime       = pingTime;								 }
	void SetExtrapolated_MoveState(ExtData::MOVESTATE movestate)		{ mCurrExtraPolated_Data.MoveState      = movestate;							 }
	
	void SetPacketPos(Vec3 pos) { mPrevPacketPos = mCurrPacketPos;  mCurrPacketPos = pos; mBezierTime = 0.f;  }

public:
	Vec3 GetDirection(Vec3 dir);
	Vec3 CalculateDirection(float yAngleRadian);
	Vec3 lerp(Vec3 CurrPos, Vec3 TargetPos, float PosLerpParam);
	Vec3 quadraticInterpolation(const Vec3& p0, const Vec3& p1, const Vec3& p2, float t);
	float GetYAngleFromQuaternion(const Vec4& rotationQuaternion);


public:

	void RotateTo(const Vec3& Angle);
	Vec3 Bezier_Curve_3(Vec3 start, Vec3 Target, float t);
	// ������ ��� ����ϴ� �Լ�
	Vec3 Script_RemotePlayer::Bezier_Curve(Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, float t);
	// ������ ��� ����ϴ� �Լ�
	Vec3 Script_RemotePlayer::Bezier_Curve_test(Vec3 p0, Vec3 p1, Vec3 p2, float t)
	{
		// ������ ��� ������ ����Ͽ� ��ġ�� ���
		float u = 1.0f - t;
		float tt = t * t;
		float uu = u * u;

		Vec3 point = (uu * p0) + (2.0f * u * t * p1) + (tt * p2);

		return point;
	}
};

