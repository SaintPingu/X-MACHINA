#include "stdafx.h"
#include "Script_NetworkPlayer.h"
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
#include "Script_Player.h"


void Script_NetworkPlayer::Awake()
{
	base::Awake();

	mLatencyTimePoint_latest = std::chrono::steady_clock::now();
	mMouseTimePoint_latest = std::chrono::steady_clock::now();
	SetClientCallback_ChangeAnimation();
}

void Script_NetworkPlayer::LateUpdate()
{
	base::LateUpdate();

	DoInput_Move();		// Send Move Transform Packet By Key Input
	DoInput_OnShoot();	// Semd Bullet On Shoot Packet By Key Input  

	DoNetLatency();

}

void Script_NetworkPlayer::UpdateData(const void* data)
{


}

void Script_NetworkPlayer::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
}

void Script_NetworkPlayer::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
}

#include <math.h>
void Script_NetworkPlayer::DoInput_Move()
{
	/// +--------------------------------------------------
	///	>> ▶▶▶▶▶ KEY PRESSED
	/// --------------------------------------------------+
	
	bool bSendPacket = false;
	bool bKeyEvent = false;

	if (KEY_PRESSED('W') || KEY_PRESSED('A') || KEY_PRESSED('S') || KEY_PRESSED('D'))
	{
		bKeyEvent = true;

		mMoveDir_Key_Pressed = GetMoveDirection_Key_Pressed();
		mMovementSpeed       = mkRunSpeed;
		/// +--------------------------------------------------------------------------------------------------------------------
		///	♣ 이동방향이 바뀌었다면 즉시 패킷을 보낸다. 
		/// _____________________________________________________________________________________________________________________
		

		if (mMoveDir_Curr != mMoveDir_Key_Pressed) {

			msendMovePacket_Pressed = true; 
			mMoveDir_Curr           = mMoveDir_Key_Pressed;
			mMoveTimePoint_latest   = std::chrono::steady_clock::now(); // 현재 시간

			Send_CPkt_Transform_Player(PLAYER_MOVE_STATE::Start, mMoveDir_Curr, mMovementSpeed);
			bSendPacket = true;
		}

		/// +--------------------------------------------------------------------------------------------------------------------
		///	♣ 이동방향이 같다면 (PlayerNetworkInfo::SendInterval_CPkt_Trnasform * 1000 ) ms시간 간격으로 보낸다.
		/// _____________________________________________________________________________________________________________________

		else if (mMoveDir_Curr == mMoveDir_Key_Pressed) {
			auto currentTime = std::chrono::steady_clock::now(); // 현재 시간
			if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mMoveTimePoint_latest).count() >= PlayerNetworkInfo::SendInterval_CPkt_Trnasform * 1000)
			{

				mMoveTimePoint_latest = currentTime;
				Send_CPkt_Transform_Player(PLAYER_MOVE_STATE::Progress, mMoveDir_Curr, mMovementSpeed);
				bSendPacket = true;

			}
		}
		//mMoveDir_Curr = mMoveDir_Key_Pressed;

	}
	mMoveDir_Curr = mMoveDir_Key_Pressed;

	/// +--------------------------------------------------
	///	>> ▶▶▶▶▶ KEY AWAY  
	/// --------------------------------------------------+
	if (KEY_AWAY('W') || KEY_AWAY('A') || KEY_AWAY('S') || KEY_AWAY('D'))
	{
		bKeyEvent = true;
		
		if (bSendPacket == false) {
			Vec3 MoveDir   = Vec3(0.f, 0.f, 0.f);
			mMovementSpeed = 0.f;

			/* 즉시 패킷을 보낸다. */
			Send_CPkt_Transform_Player(PLAYER_MOVE_STATE::End, mMoveDir_Curr, mMovementSpeed);

			/* 움직임 상태 초기화 */
			mMoveDir_Key_Pressed = MoveDir;
			mMoveDir_Key_Tap     = MoveDir;

		}
	}

	if (!bKeyEvent) {

		auto currentTime = std::chrono::steady_clock::now(); // 현재 시간
		if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mDefaultTimePoint_latest).count() >= 0.5f * 1000) {
			/* 아무 입력키를 누르고 있지 않을 때 1초에 2번씩 패킷을 보내자. */
			Send_CPkt_Transform_Player(PLAYER_MOVE_STATE::Default, Vec3(0.f, 0.f, 0.f), 0.05F);
			mDefaultTimePoint_latest = currentTime;
		}

	}

	return;
}

void Script_NetworkPlayer::DoInput_OnShoot()
{


}

Vec3 Script_NetworkPlayer::GetMoveDirection_Key_Tap()
{
	Dir MoveDirection = Dir::None;
	float v{};
	float h{};

	// 움직임 방향을 구한다.
	if (KEY_TAP('W')) { v += 1; }
	if (KEY_TAP('S')) { v -= 1; }
	if (KEY_TAP('A')) { h -= 1; }
	if (KEY_TAP('D')) { h += 1; }

	MoveDirection |= Math::IsZero(v) ? Dir::None : (v > 0) ? Dir::Front : Dir::Back;
	MoveDirection |= Math::IsZero(h) ? Dir::None : (h > 0) ? Dir::Right : Dir::Left;

	Vec3 MoveDir = Transform::GetWorldDirection(MoveDirection);
	return MoveDir;
}

Vec3 Script_NetworkPlayer::GetMoveDirection_Key_Pressed()
{
	Dir MoveDirection = Dir::None;
	float v{};
	float h{};

	// 움직임 방향을 구한다.
	if (KEY_PRESSED('W')) { v += 1; }
	if (KEY_PRESSED('S')) { v -= 1; }
	if (KEY_PRESSED('A')) { h -= 1; }
	if (KEY_PRESSED('D')) { h += 1; }

	MoveDirection |= Math::IsZero(v) ? Dir::None : (v > 0) ? Dir::Front : Dir::Back;
	MoveDirection |= Math::IsZero(h) ? Dir::None : (h > 0) ? Dir::Right : Dir::Left;

	Vec3 MoveDir = Transform::GetWorldDirection(MoveDirection);
	return MoveDir;
}

Vec3 Script_NetworkPlayer::GetmoveDirection_Key_Away()
{
	Dir MoveDirection = Dir::None;
	float v{};
	float h{};

	// 움직임 방향을 구한다.
	if (KEY_AWAY('W')) { v += 1; }
	if (KEY_AWAY('S')) { v -= 1; }
	if (KEY_AWAY('A')) { h -= 1; }
	if (KEY_AWAY('D')) { h += 1; }

	MoveDirection |= Math::IsZero(v) ? Dir::None : (v > 0) ? Dir::Front : Dir::Back;
	MoveDirection |= Math::IsZero(h) ? Dir::None : (h > 0) ? Dir::Right : Dir::Left;

	Vec3 MoveDir = Transform::GetWorldDirection(MoveDirection);
	return MoveDir;
}

void Script_NetworkPlayer::UpdateMovement(Dir dir)
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

void Script_NetworkPlayer::SetState(PlayerMotion prevState, PlayerMotion prevMotion, PlayerMotion crntState)
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

void Script_NetworkPlayer::DoNetLatency()
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

float Script_NetworkPlayer::GetYRotation()
{
	return Vector3::SignedAngle(Vector3::Forward, mObject->GetLook().xz(), Vector3::Up);
}

void Script_NetworkPlayer::SetClientCallback_ChangeAnimation()
{
	const auto& controller = mObject->GetObj<GameObject>()->GetAnimator()->GetController();

	controller->SetAnimationSendCallback(std::bind(&Script_NetworkPlayer::ClientCallBack_ChangeAnimation, this));
	controller->SetPlayer();
}

void Script_NetworkPlayer::ClientCallBack_ChangeAnimation()
{
	const auto& controller = mObject->GetObj<GameObject>()->GetAnimator()->GetController();

	int anim_upper_idx = controller->GetMotionIndex(0);
	int anim_lower_idx = controller->GetMotionIndex(1);
	float v = controller->GetParam("Vertical")->val.f;
	float h = controller->GetParam("Horizontal")->val.f;

	/* Send Changed Animation Packet To Server */
	auto pkt = FBS_FACTORY->CPkt_Player_Animation(anim_upper_idx, anim_lower_idx, h, v);
	CLIENT_NETWORK->Send(pkt);
}

void Script_NetworkPlayer::Send_CPkt_Transform_Player(int32_t moveState, Vec3 moveDir, float speed)
{
	Vec3		Pos          = GameFramework::I->GetPlayer()->GetPosition();
	float		y_rot		 = GetYRotation();
	Vec3		Rot			 = Vec3(0.f, y_rot, 0.f);
	Vec3		SpineDir     = GameFramework::I->GetPlayer()->GetComponent<Script_GroundPlayer>()->GetSpineBone()->GetLook();
	long long	latency      = FBS_FACTORY->CurrLatency.load();

	const auto& controller  = mObject->GetObj<GameObject>()->GetAnimator()->GetController();
	float		animparam_h = controller->GetParam("Horizontal")->val.f;
	float		animparam_v = controller->GetParam("Vertical")->val.f;

	auto pkt = FBS_FACTORY->CPkt_Player_Transform(Pos, Rot, moveState, moveDir, speed, SpineDir, latency, animparam_h, animparam_v);
	CLIENT_NETWORK->Send(pkt);
}

void Script_NetworkPlayer::Send_CPkt_AimRotation_Player(float aim_rotation_y)
{
	auto pkt = FBS_FACTORY->CPkt_Player_AimRotation(aim_rotation_y);
	CLIENT_NETWORK->Send(pkt);
}

