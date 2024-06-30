#pragma once

#include "Script_Network.h"

class GridObject;
class AnimatorController;

namespace PlayerNetworkInfo
{
	constexpr float SendInterval_CPkt_Trnasform      = 1.f / 20.f; // 1s에 16번 간격으로 CPkt_Transform 전송
	constexpr float sendInterval_CPkt_NetworkLatency = 1.f / 10.f; // 1s에 10번 간격으로 CPkt_NetworkLatency 전송

}

class Script_PlayerNetwork : public Script_Network{
	COMPONENT(Script_PlayerNetwork, Script_Network)

private:
	std::chrono::steady_clock::time_point mMoveTimePoint_latest = {};
	std::chrono::steady_clock::time_point mLatencyTimePoint_latest = {};

	Vec3 mPrevPos;

public:
	virtual void Awake() override;
	virtual void LateUpdate() override;
	virtual void UpdateData(const void* data) override;

private:
	void DoInput();

	void DoNetLatency();

public:	
	float GetYRotation();

private:
	void SetClientCallback_ChangeAnimation();
	void ClientCallBack_ChangeAnimation(); // AnimationController 에서 Animation이 바뀌었을 때 이 함수가 호출된다. 


public:
	void Send_CPkt_Transform_Player(int32_t moveState);

};

