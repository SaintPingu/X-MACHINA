#pragma once

/// +-----------------------------------------------
///					FBsPacketFactory 
/// ________________________________________________
/// 
/// 클라이언트로 부터 받은 패킷을 해석하는 역할을 담당한다. 
/// 
/// -----------------------------------------------+
#undef max 
#include <flatbuffers/flatbuffers.h>
#include "ClientNetwork/Include/Protocol/FBProtocol_generated.h"
#include "GamePlayer.h"
#include "InputMgr.h"

#define FBS_FACTORY FBsPacketFactory::GetInst()
class FBsPacketFactory
{
	DECLARE_SINGLETON(FBsPacketFactory);
private:
	static std::atomic_int CurrLatency;
	static std::atomic<long long> TotalLatency;
	static std::atomic<int>		  LatencyCount; 

public:
	static bool ProcessFBsPacket(SPtr_Session session, BYTE* packetBuf, UINT32 Datalen);

private:
	/* ※ INVALID ※ */
	static bool Process_SPkt_Invalid(SPtr_Session session, BYTE* packetBuf, UINT32 Datalen);
	static bool Process_SPkt_LogIn(SPtr_Session session, const FBProtocol::SPkt_LogIn& pkt);
	static bool Process_SPkt_EnterGame(SPtr_Session session, const FBProtocol::SPkt_EnterGame& pkt);
	static bool Process_SPkt_NewPlayer(SPtr_Session session, const FBProtocol::SPkt_NewPlayer& pkt);
	static bool Process_SPkt_RemovePlayer(SPtr_Session session, const FBProtocol::SPkt_RemovePlayer& pkt);
	static bool Process_SPkt_Transform(SPtr_Session session, const FBProtocol::SPkt_Transform& pkt);
	static bool Process_SPkt_KeyInput(SPtr_Session session, const FBProtocol::SPkt_KeyInput& pkt);
	static bool Process_SPkt_PlayerState(SPtr_Session session, const FBProtocol::SPkt_PlayerState& pkt);
	static bool Process_SPkt_NetworkLatency(SPtr_Session session, const FBProtocol::SPkt_NetworkLatency& pkt);
	static bool Process_SPkt_Chat(SPtr_Session session, const FBProtocol::SPkt_Chat& pkt);

public:
	SPtr_SendPktBuf CPkt_Chat(UINT32 sessionID, std::string msg);
	SPtr_SendPktBuf CPkt_NewtorkLatency(long long timestamp);
	SPtr_SendPktBuf CPkt_LogIn();
	SPtr_SendPktBuf CPkt_EnterGame(uint64_t playerIdx);
	SPtr_SendPktBuf CPkt_NewPlayer();
	SPtr_SendPktBuf CPkt_RemovePlayer(int removeSessionID);
	SPtr_SendPktBuf CPkt_KeyInput(GameKeyInfo::KEY key, GameKeyInfo::KEY_STATE KeyState, GameKeyInfo::MoveKey moveKey, Vec2 mouseDelta);
	SPtr_SendPktBuf CPkt_Transform(Vec3 Pos, Vec3 Rot, Vec3 Scale, Vec3 SpineLookDir, long long timestamp);

private:
	static GamePlayerInfo GetPlayerInfo(const FBProtocol::Player* player);
	static Vec3 GetVector3(const FBProtocol::Vector3* vec3);

};

