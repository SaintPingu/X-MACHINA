#include "stdafx.h"
#include "Script_PlayerNetwork.h"
#include "GameFramework.h"
#include "X-Engine.h"
#include "Object.h"
#include "Animator.h"

#include "Script_GroundObject.h"
#include "Script_Player.h"


void Script_PlayerNetwork::Awake()
{
	base::Awake();


}

void Script_PlayerNetwork::LateUpdate()
{
	base::LateUpdate();

	///* Player Network ���� ����� ����ϴ� Script�� ���� ���� .. */
	//if (KEY_TAP('W') || KEY_TAP('A') || KEY_TAP('S') || KEY_TAP('D'))
	//{
	//	mMoveTimePoint_latest = std::chrono::steady_clock::now();
	//	Vec3 Pos              = GameFramework::I->GetPlayer()->GetPosition();
	//	Vec3 Rot              = GameFramework::I->GetPlayer()->GetRotation();
	//	Vec3 Sca              = Vec3(1.f, 1.f, 1.f);
	//	Vec3 SpineDir         = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetSpineBone()->GetLook();

	//	long long timestamp = NETWORK_MGR->GetTimeStamp();
	//	NETWORK_MGR->Send_CPkt_Transform(Pos, Rot, Sca, SpineDir, timestamp);

	//}
	//if (KEY_PRESSED('W') || KEY_PRESSED('A') || KEY_PRESSED('S') || KEY_PRESSED('D'))
	//{
	//	/* 1s�� PlayerNetworkInfo::SendInterval_CPkt_Trnasform ���� ��ŭ ��Ŷ�� ������. */
	//	auto currentTime = std::chrono::steady_clock::now(); // ���� �ð�
	//	if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mMoveTimePoint_latest).count()
	//		>= PlayerNetworkInfo::SendInterval_CPkt_Trnasform * 1000)
	//	{
	//		mMoveTimePoint_latest = currentTime;
	//		Vec3 Pos              = GameFramework::I->GetPlayer()->GetPosition();
	//		Vec3 Rot              = GameFramework::I->GetPlayer()->GetRotation();
	//		Vec3 Sca              = Vec3(1.f, 1.f, 1.f);
	//		Vec3 SpineDir         = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetSpineBone()->GetLook();

	//		long long timestamp = NETWORK_MGR->GetTimeStamp();
	//		NETWORK_MGR->Send_CPkt_Transform(Pos, Rot, Sca, SpineDir, timestamp);
	//	}
	//}
	//if (KEY_AWAY('W') || KEY_AWAY('A') || KEY_AWAY('S') || KEY_AWAY('D'))
	//{
	//	Vec3 Pos      = GameFramework::I->GetPlayer()->GetPosition();
	//	Vec3 Rot      = GameFramework::I->GetPlayer()->GetRotation();
	//	Vec3 Sca      = Vec3(1.f, 1.f, 1.f);
	//	Vec3 SpineDir = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetSpineBone()->GetLook();

	//	long long timestamp = NETWORK_MGR->GetTimeStamp();
	//	NETWORK_MGR->Send_CPkt_Transform(Pos, Rot, Sca, SpineDir, timestamp);

	//}

}

void Script_PlayerNetwork::UpdateData(const void* data)
{


}

ExtData Script_PlayerNetwork::Extrapolation(ExtData d0, ExtData d1)
{
	/* latency */
	//long long Pkt_latency = d1.timestamp - d0.timestamp; /* �̶� timestamp�� ��Ŷ���� ���� Timestamp�� �ƴϴ�! ���� ���� �� ���� timestamp ( �� ���� �ð�.. why? �� Ŭ�󸶴� �ð��� �ٸ���! ) */

	/* Extrapolated Pos */
	//Vec3 Extrapolated_Pos = (d1.pos - d0.pos).Normalize() * ; /* (d1.pos - d0.pos).normalize * velocity * latency /

	/* Extrapo;ated Rot */
	//int RotDir = (d1.Rot - d0.Rot) < 0 ? -1 : 1;
	//Vec3 Extrapolated_Rot; /* (d1.Rot - d0.Rot = -1 ? 1) * Rotation Speed * latency */

	return ExtData();
}

