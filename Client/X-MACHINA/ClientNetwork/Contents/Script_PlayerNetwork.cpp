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
	//DoNetLatency();

}

void Script_PlayerNetwork::UpdateData(const void* data)
{


}

#include <math.h>
void Script_PlayerNetwork::DoInput()
{

	Dir dir;
	float v{}, h{};
	if (KEY_PRESSED('W')) { v += 1; }
	if (KEY_PRESSED('S')) { v -= 1; }
	if (KEY_PRESSED('A')) { h -= 1; }
	if (KEY_PRESSED('D')) { h += 1; }

	dir |= Math::IsZero(v) ? Dir::None : (v > 0) ? Dir::Front : Dir::Back;
	dir |= Math::IsZero(h) ? Dir::None : (h > 0) ? Dir::Right : Dir::Left;

	UpdateMovement(dir);

	const auto& controller = mObject->GetObj<GameObject>()->GetAnimator()->GetController();

	auto pkt = FBS_FACTORY->CPkt_Player_Transform(/* POSITION  */ GameFramework::I->GetPlayer()->GetPosition(),
												  /* ROTATION  */ Vec3(0.f, GetYRotation(), 0.f),
												  /* MOVESATE  */ PLAYER_MOVE_STATE::Start,
												  /* MOVEDIR   */ Transform::GetWorldDirection(dir),
												  /* MOVESPEED */ mMovementSpeed,
												  /* LOOK      */ GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetSpineBone()->GetLook(),
												  /* LATENCY   */ FBS_FACTORY->CurrLatency.load(),
												  /* ANIM _H   */ controller->GetParam("Horizontal")->val.f,
												  /* ANiM _F   */ controller->GetParam("Vertical")->val.f);
	CLIENT_NETWORK->Send(pkt);

	return;

	///* Player Network 관련 기능을 담당하는 Script에 넣을 예정 .. */
	if (KEY_TAP('W') || KEY_TAP('A') || KEY_TAP('S') || KEY_TAP('D'))
	{
		Vec3		MoveDir; //= //Pos - mPrevPos; MoveDir.Normalize();
		if (KEY_TAP('W')) MoveDir = Vec3(0.f, 0.f, 1.f);
		if (KEY_TAP('A')) MoveDir = Vec3(-1.f, 0.f, 0.f);
		if (KEY_TAP('S')) MoveDir = Vec3(0.f, 0.f, -1.f);
		if (KEY_TAP('D')) MoveDir = Vec3(1.f, 0.f, 0.f);



		mMoveTimePoint_latest = std::chrono::steady_clock::now();
		//Send_CPkt_Transform_Player(PLAYER_MOVE_STATE::Start);

		float		Vel		 = 5.f; // GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetMovementSpeed();
		Vec3		Pos      = GameFramework::I->GetPlayer()->GetPosition();
		//Vec3		MoveDir  = //Pos - mPrevPos; MoveDir.Normalize();
		float		y_rot    = GetYRotation();
		Vec3		Rot      = Vec3(0.f, y_rot, 0.f);
		Vec3		SpineDir = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetSpineBone()->GetLook();
		long long	latency  = FBS_FACTORY->CurrLatency.load();

		const auto& controller = mObject->GetObj<GameObject>()->GetAnimator()->GetController();
		float					animparam_h = controller->GetParam("Horizontal")->val.f;
		float					animparam_v = controller->GetParam("Vertical")->val.f;

		auto pkt = FBS_FACTORY->CPkt_Player_Transform(Pos, Rot, PLAYER_MOVE_STATE::Start, MoveDir, Vel, SpineDir, latency, animparam_h, animparam_v);
		CLIENT_NETWORK->Send(pkt);

		//std::cout << "SEND TRAN CPKT : START\n";
	}

	if (KEY_PRESSED('W') || KEY_PRESSED('A') || KEY_PRESSED('S') || KEY_PRESSED('D'))
	{
		/* 1s에 PlayerNetworkInfo::SendInterval_CPkt_Trnasform 개수 만큼 패킷을 보낸다. */
		auto currentTime = std::chrono::steady_clock::now(); // 현재 시간
		if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mMoveTimePoint_latest).count()
			>= PlayerNetworkInfo::SendInterval_CPkt_Trnasform * 1000)
		{
			mMoveTimePoint_latest = currentTime;
			Send_CPkt_Transform_Player(PLAYER_MOVE_STATE::Progress);
			//std::cout << "SEND TRAN CPKT : PROGRESS\n";
		}
	}

	if (KEY_AWAY('W') || KEY_AWAY('A') || KEY_AWAY('S') || KEY_AWAY('D'))
	{
		Send_CPkt_Transform_Player(PLAYER_MOVE_STATE::End);
		//std::cout << "SEND TRAN CPKT : END\n";
	}

	if (GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetMoveDir() !=
		GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetPrevMoveDir())
	{
		//mMoveTimePoint_latest = std::chrono::steady_clock::now();
		//Send_CPkt_Transform_Player(PLAYER_MOVE_STATE::Start);
	}
}

void Script_PlayerNetwork::UpdateMovement(Dir dir)
{	
	// 현재 캐릭터의 움직임 상태를 키 입력에 따라 설정한다.
	PlayerMotion crntMovement = PlayerMotion::None;
	// Stand / Sit
	if (KEY_PRESSED(VK_CONTROL)) { crntMovement |= PlayerMotion::Sit; }
	else { crntMovement |= PlayerMotion::Stand; }
	// Walk / Run / Sprint
	if (dir != Dir::None) {
		if (mIsAim) {
			crntMovement |= PlayerMotion::Walk;
		}
		else {
			if (KEY_PRESSED(VK_SHIFT)) { crntMovement |= PlayerMotion::Sprint; }
			else if (KEY_PRESSED('C')) { crntMovement |= PlayerMotion::Walk; }
			else { crntMovement |= PlayerMotion::Run; }
		}
	}

	PlayerMotion prevState = PlayerMotion::GetState(mPrevMovement);
	PlayerMotion prevMotion = PlayerMotion::GetMotion(mPrevMovement);

	PlayerMotion crntState = PlayerMotion::GetState(crntMovement);
	PlayerMotion crntMotion = PlayerMotion::GetMotion(crntMovement);

	SetState(prevState, prevMotion, crntState);

	mPrevMovement = crntState | crntMotion;
}

void Script_PlayerNetwork::SetState(PlayerMotion prevState, PlayerMotion prevMotion, PlayerMotion crntState)
{	// 이전 움직임 상태와 다른 경우만 값을 업데이트 한다.
	// 이전 상태를 취소하고 현재 상태로 전환한다.
	if (!(crntState & prevState)) {
		switch (prevState) {
		case PlayerMotion::None:
		case PlayerMotion::Stand:
			break;
		case PlayerMotion::Sit:
			break;

		default:
			assert(0);
			break;
		}

		switch (crntState) {
		case PlayerMotion::None:
			break;
		case PlayerMotion::Stand:
		{
			switch (prevMotion) {
			case PlayerMotion::None:
				break;
			case PlayerMotion::Walk:
				mMovementSpeed = mkStandWalkSpeed;
				break;
			case PlayerMotion::Run:
				mMovementSpeed = mkRunSpeed;
				break;
			case PlayerMotion::Sprint:
				mMovementSpeed = mkSprintSpeed;
				break;
			default:
				assert(0);
				break;
			}
		}
		break;
		case PlayerMotion::Sit:
			mMovementSpeed = mkSitWalkSpeed;
			break;

		default:
			assert(0);
			break;
		}
	}
}

void Script_PlayerNetwork::DoNetLatency()
{
	/* 1초에 10번 Latency 패킷을 측정한다. */

	/* 1s에 PlayerNetworkInfo::SendInterval_CPkt_NetworkLateny 개수 만큼 패킷을 보낸다. */
	auto currentTime = std::chrono::steady_clock::now(); // 현재 시간
	if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mLatencyTimePoint_latest).count()
		>= PlayerNetworkInfo::sendInterval_CPkt_NetworkLatency * 1000) {

		mLatencyTimePoint_latest = currentTime;

		long long timeStamp = CLIENT_NETWORK->GetCurrentTimeMilliseconds();
		auto pkt = FBS_FACTORY->CPkt_NetworkLatency(timeStamp);
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
	auto pkt = FBS_FACTORY->CPkt_Player_Animation(anim_upper_idx, anim_lower_idx, h, v);
	CLIENT_NETWORK->Send(pkt);
}

void Script_PlayerNetwork::Send_CPkt_Transform_Player(int32_t moveState)
{
	float		Vel          = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetMovementSpeed();
	Vec3		Pos          = GameFramework::I->GetPlayer()->GetPosition();
	Vec3		MoveDir      = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetMoveDir();//Pos - mPrevPos; MoveDir.Normalize();
	float		y_rot		 = GetYRotation();
	Vec3		Rot			 = Vec3(0.f, y_rot, 0.f);
	Vec3		SpineDir     = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetSpineBone()->GetLook();
	long long	latency      = FBS_FACTORY->CurrLatency.load();

	const auto& controller = mObject->GetObj<GameObject>()->GetAnimator()->GetController();
	float					animparam_h = controller->GetParam("Horizontal")->val.f;
	float					animparam_v = controller->GetParam("Vertical")->val.f;

	auto pkt = FBS_FACTORY->CPkt_Player_Transform(Pos, Rot, moveState, MoveDir, Vel, SpineDir, latency, animparam_h, animparam_v);
	CLIENT_NETWORK->Send(pkt);
}

