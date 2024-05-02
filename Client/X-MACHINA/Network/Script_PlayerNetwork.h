#pragma once

#include "Script_Network.h"

/* Extrapolation Data - 패킷을 통한 위치 예측을 위해 필요한 데이터 */
struct ExtData
{
	long long timestamp{};
	Vec3	  pos{};
	Vec3	  Rot{};


};

namespace PlayerNetworkInfo
{
	constexpr float SendInterval_CPkt_Trnasform = 1.f / 16.f; // 16s 간격으로 CPkt_Transform 전송
}

class Script_PlayerNetwork : public Script_Network{
	COMPONENT(Script_PlayerNetwork, Script_Network)

private:
	std::chrono::steady_clock::time_point mMoveTimePoint_latest = {};


public:
	virtual void Awake() override;
	virtual void LateUpdate() override;
	virtual void UpdateData(const void* data) override;


public:
	/* 데드 레커닝 */
	ExtData Extrapolation(ExtData d0, ExtData d1); /* 플레이어 위치, 회전 예측 */


};

