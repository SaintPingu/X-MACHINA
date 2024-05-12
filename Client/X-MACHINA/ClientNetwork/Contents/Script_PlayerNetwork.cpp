#include "stdafx.h"
#include "Script_PlayerNetwork.h"
#include "ClientNetworkManager.h"
#include "FBsPacketFactory.h"

#include "GameFramework.h"
#include "X-Engine.h"
#include "Object.h"
#include "FBsPacketFactory.h"
#include "Animator.h"
#include "AnimatorController.h"
#include "Object.h"

#include "InputMgr.h"
#include "Script_GroundObject.h"
#include "Script_Player.h"


void Script_PlayerNetwork::Awake()
{
	base::Awake();

	mLatencyTimePoint_latest = std::chrono::steady_clock::now();
	SetClientCallback_ChangeAnimation();
}

void Script_PlayerNetwork::LateUpdate()
{
	base::LateUpdate();

	DoInput();
	DoNetLatency();

}

void Script_PlayerNetwork::UpdateData(const void* data)
{


}

#include <math.h>
void Script_PlayerNetwork::DoInput()
{
	///* Player Network ���� ����� ����ϴ� Script�� ���� ���� .. */
	if (KEY_TAP('W') || KEY_TAP('A') || KEY_TAP('S') || KEY_TAP('D'))
	{
		mMoveTimePoint_latest = std::chrono::steady_clock::now();
		
		float					Vel        = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetMovementSpeed();
		Vec3					Pos        = GameFramework::I->GetPlayer()->GetPosition();
		Vec3					MoveDir    = Pos - mPrevPos; MoveDir.Normalize();
		
		Vec4					QuatRot = GameFramework::I->GetPlayer()->GetRotation();
		Vec3 Rot{};  Rot.y = GetYRotation();

		int32_t	moveState = PLAYER_MOVE_STATE::Start;
		Vec3					SpineDir   = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetSpineBone()->GetLook();
		long long				latency    = FBS_FACTORY->CurrLatency.load();

		auto pkt = FBS_FACTORY->CPkt_Transform(Pos, Rot, moveState, MoveDir, Vel, SpineDir, latency, 0, 0);
		CLIENT_NETWORK->Send(pkt);

		mPrevPos = Pos;
	}

	if (KEY_PRESSED('W') || KEY_PRESSED('A') || KEY_PRESSED('S') || KEY_PRESSED('D'))
	{
		/* 1s�� PlayerNetworkInfo::SendInterval_CPkt_Trnasform ���� ��ŭ ��Ŷ�� ������. */
		auto currentTime = std::chrono::steady_clock::now(); // ���� �ð�
		if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mMoveTimePoint_latest).count()
			>= PlayerNetworkInfo::SendInterval_CPkt_Trnasform * 1000)
		{
			mMoveTimePoint_latest = currentTime;

			float					Vel       = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetMovementSpeed();
			Vec3					Pos       = GameFramework::I->GetPlayer()->GetPosition();
			Vec3					MoveDir   = Pos - mPrevPos; MoveDir.Normalize();
			Vec4					QuatRot       = GameFramework::I->GetPlayer()->GetRotation();
			Vec3 Rot{};  Rot.y = GetYRotation();

			int32_t	moveState = PLAYER_MOVE_STATE::Progress;
			Vec3					SpineDir  = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetSpineBone()->GetLook();
			long long				latency   = FBS_FACTORY->CurrLatency.load();

			const auto& controller = mObject->GetObj<GameObject>()->GetAnimator()->GetController();
			float					animparam_h = controller->GetParam("Horizontal")->val.f;
			float					animparam_v = controller->GetParam("Vertical")->val.f;

			auto pkt = FBS_FACTORY->CPkt_Transform(Pos, Rot, moveState, MoveDir, Vel, SpineDir, latency, animparam_h, animparam_v);
			CLIENT_NETWORK->Send(pkt);

			mPrevPos = Pos;

		}
	}

	if (KEY_AWAY('W') || KEY_AWAY('A') || KEY_AWAY('S') || KEY_AWAY('D'))
	{
		float					Vel       = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetMovementSpeed();
		Vec3					Pos       = GameFramework::I->GetPlayer()->GetPosition();
		Vec3					MoveDir   = Pos - mPrevPos; MoveDir.Normalize();
		Vec4					QuatRot = GameFramework::I->GetPlayer()->GetRotation();
		Vec3 Rot{};  Rot.y = GetYRotation();

		int32_t	moveState = PLAYER_MOVE_STATE::End;
		Vec3					SpineDir  = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetSpineBone()->GetLook();
		long long				latency   = FBS_FACTORY->CurrLatency.load();

		auto pkt = FBS_FACTORY->CPkt_Transform(Pos, Rot, moveState, MoveDir, Vel, SpineDir, latency, 0, 0);
		CLIENT_NETWORK->Send(pkt);

		mPrevPos = Pos;

	}
}

void Script_PlayerNetwork::DoNetLatency()
{
	/* 1�ʿ� 10�� Latency ��Ŷ�� �����Ѵ�. */

	/* 1s�� PlayerNetworkInfo::SendInterval_CPkt_NetworkLateny ���� ��ŭ ��Ŷ�� ������. */
	auto currentTime = std::chrono::steady_clock::now(); // ���� �ð�
	if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mLatencyTimePoint_latest).count()
		>= PlayerNetworkInfo::sendInterval_CPkt_NetworkLatency * 1000) {

		mLatencyTimePoint_latest = currentTime;

		long long timeStamp = CLIENT_NETWORK->GetCurrentTimeMilliseconds();
		auto pkt = FBS_FACTORY->CPkt_NewtorkLatency(timeStamp);
		CLIENT_NETWORK->Send(pkt);

	}
}

float Script_PlayerNetwork::GetYRotation()
{
	return Vector3::SignedAngle(Vector3::Forward, mObject->GetLook().xz(), Vector3::Up);
}

void Script_PlayerNetwork::SetClientCallback_ChangeAnimation()
{
	const auto& controller = mObject->GetObj<GameObject>()->GetAnimator()->GetController();

	controller->SetAnimationSendCallback(std::bind(&Script_PlayerNetwork::ClientCallBack_ChangeAnimation, this));
	controller->SetPlayer();
}

void Script_PlayerNetwork::ClientCallBack_ChangeAnimation()
{
	const auto& controller = mObject->GetObj<GameObject>()->GetAnimator()->GetController();

	int anim_upper_idx = controller->GetMotionIndex("Body");
	int anim_lower_idx = controller->GetMotionIndex("Legs");
	float v = controller->GetParam("Vertical")->val.f;
	float h = controller->GetParam("Horizontal")->val.f;

	/* Send Changed Animation Packet To Server */
	auto pkt = FBS_FACTORY->CPkt_PlayerAnimation(anim_upper_idx, anim_lower_idx, h, v);
	CLIENT_NETWORK->Send(pkt);
}
