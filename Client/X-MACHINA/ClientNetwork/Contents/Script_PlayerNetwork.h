#pragma once

#include "Script_Network.h"

/* Extrapolation Data - ��Ŷ�� ���� ��ġ ������ ���� �ʿ��� ������ */
struct ExtData
{
	long long timestamp{};
	Vec3	  pos{};
	Vec3	  Rot{};


};

namespace PlayerNetworkInfo
{
	constexpr float SendInterval_CPkt_Trnasform = 1.f / 16.f; // 16s �������� CPkt_Transform ����
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
	/* ���� ��Ŀ�� */
	ExtData Extrapolation(ExtData d0, ExtData d1); /* �÷��̾� ��ġ, ȸ�� ���� */


};

