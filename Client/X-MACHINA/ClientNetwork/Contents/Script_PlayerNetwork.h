#pragma once

#include "Script_Network.h"

class GridObject;
class AnimatorController;

namespace PlayerNetworkInfo
{
	constexpr float SendInterval_CPkt_Trnasform      = 1.f / 30.f; // 1s�� 16�� �������� CPkt_Transform ����
	constexpr float sendInterval_CPkt_NetworkLatency = 1.f / 10.f; // 1s�� 10�� �������� CPkt_NetworkLatency ����

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
	void ClientCallBack_ChangeAnimation(); // AnimationController ���� Animation�� �ٲ���� �� �� �Լ��� ȣ��ȴ�. 

};

