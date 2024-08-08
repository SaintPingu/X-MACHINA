#pragma once

#include "Script_NetworkLiveObject.h"

class Script_GroundPlayer;

namespace PlayerNetworkInfo
{
	constexpr float SendInterval_CPkt_Trnasform			 = 1.f / 10.f; // 1s에 CPkt_Transform 전송
	constexpr float SendInterval_CPkt_MouseAimRotation   = 1.f / 40.f; // 1s에 16번 간격으로 CPkt_Transform 전송
	constexpr float sendInterval_CPkt_NetworkLatency	 = 1.f / 10.f; // 1s에 10번 간격으로 CPkt_NetworkLatency 전송

}

class Script_NetworkPlayer : public Script_NetworkLiveObject {
	COMPONENT(Script_NetworkPlayer, Script_NetworkLiveObject)

private:
	std::chrono::steady_clock::time_point mMoveTimePoint_latest    = {};
	std::chrono::steady_clock::time_point mDefaultTimePoint_latest = {};
	std::chrono::steady_clock::time_point mMouseTimePoint_latest   = {}; /* Mouse Interval Time */
	std::chrono::steady_clock::time_point mLatencyTimePoint_latest = {};

	Script_GroundPlayer* mPlayer{};
	Transform* mSpineBone{};
	Vec3			mPrevPos;
	PlayerMotion	mMotion;	
	PlayerMotion	mPrevMovement;
	bool			mIsAim{};

	float			mMovementSpeed		= 0.f;
	
	float			mkSitWalkSpeed		= 1.5f;
	float			mkStandWalkSpeed	= 2.2f;
	float			mkRunSpeed			= 5.f;
	float			mkSprintSpeed		= 8.f;
	float			mkStartRotAngle		= 40.f;

private:
	/* Move Direction ( Normalized ) */
	Vec3			mMoveDir_Curr{};
	Vec3			mMoveDir_Key_Tap{};
	Vec3			mMoveDir_Key_Pressed{};

	bool			msendMovePacket_Pressed = false;

	float			mPrevAngle_y = 0.f;

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void LateUpdate() override;
	virtual void UpdateData(const void* data) override;

	virtual void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
	virtual void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam);

private:

	
	void DoInput_Move();
	void DoInput_Mouse();
	void DoInput_OnShoot();
	void DoNetLatency();

	Vec3 GetMoveDirection_Key_Tap();
	Vec3 GetMoveDirection_Key_Pressed();
	Vec3 GetmoveDirection_Key_Away();

public:
	void Send_CPkt_Transform_Player(int32_t moveState, Vec3 moveDir, float speed);
	void Send_CPkt_AimRotation_Player(float aim_rotation_y, float spine_angle);

	void SetState(PlayerMotion prevState, PlayerMotion prevMotion, PlayerMotion crntState);
	void UpdateMovement(Dir dir);
	void ClientCallBack_ChangeAnimation(); // AnimationController 에서 Animation이 바뀌었을 때 이 함수가 호출된다. 
	void SetClientCallback_ChangeAnimation();
	float GetYRotation();
};

