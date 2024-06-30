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
#include "ClientNetwork/Include/Protocol/Enum_generated.h"
#include "ClientNetwork/Include/Protocol/Transform_generated.h"
#include "ClientNetwork/Include/Protocol/Struct_generated.h"

#include "NetworkEvents.h"
#include "GamePlayer.h"
#include "InputMgr.h"

namespace PLAYER_MOVE_STATE
{
	constexpr int32_t Start = 0;
	constexpr int32_t Progress = 1;
	constexpr int32_t End = 2;
}

#define FBS_FACTORY FBsPacketFactory::GetInst()
class FBsPacketFactory
{
	DECLARE_SINGLETON(FBsPacketFactory);
public:
	static std::atomic<long long>	CurrLatency; //  TotalLatency / LatencyCount(10) 1초 기준 Latency 평균을 저장 
private:
	static std::atomic<long long> TotalLatency;
	static std::atomic<int>		  LatencyCount;

public:
	static bool ProcessFBsPacket(SPtr_Session session, BYTE* packetBuf, UINT32 Datalen);

private:
	/// +------------------------
	///	  PROCESS SERVER PACKET 
	/// ------------------------+
	/* INVALID, LOGIN, ENTER_GAME, LATENCY, CHAT */
	static bool Process_SPkt_Invalid(SPtr_Session session, BYTE* packetBuf, UINT32 Datalen);
	static bool Process_SPkt_LogIn(SPtr_Session session, const FBProtocol::SPkt_LogIn& pkt);
	static bool Process_SPkt_EnterGame(SPtr_Session session, const FBProtocol::SPkt_EnterGame& pkt);
	static bool Process_SPkt_NetworkLatency(SPtr_Session session, const FBProtocol::SPkt_NetworkLatency& pkt);
	static bool Process_SPkt_Chat(SPtr_Session session, const FBProtocol::SPkt_Chat& pkt);
	
	/* PLAYER */
	static bool Process_SPkt_NewPlayer(SPtr_Session session, const FBProtocol::SPkt_NewPlayer& pkt);
	static bool Process_SPkt_RemovePlayer(SPtr_Session session, const FBProtocol::SPkt_RemovePlayer& pkt);
	static bool Process_SPkt_Player_Transform(SPtr_Session session, const FBProtocol::SPkt_Player_Transform& pkt);
	static bool Process_SPkt_Player_Animation(SPtr_Session session, const FBProtocol::SPkt_Player_Animation& pkt);
	static bool Process_SPkt_Player_Weapon(SPtr_Session session, const FBProtocol::SPkt_Player_Weapon& pkt);
	
	/* MONSTER */
	static bool Process_SPkt_Monster_Transform(SPtr_Session session, const FBProtocol::SPkt_Monster_Transform& pkt);
	static bool Process_SPkt_Monster_HP(SPtr_Session session, const FBProtocol::SPkt_Monster_HP& pkt);
	static bool Process_SPkt_Monster_State(SPtr_Session session, const FBProtocol::SPkt_Monster_State& pkt);
	
	/* BULLET */
	static bool Process_SPkt_Bullet_OnShoot(SPtr_Session session, const FBProtocol::SPkt_Bullet_OnShoot& pkt);
	static bool Process_SPkt_Bullet_OnCollision(SPtr_Session session, const FBProtocol::SPkt_Bullet_OnCollision& pkt);

public:
	/// +------------------------
	///	  CREATE CLIENT PACKET  
	/// ------------------------+
	/* LOGIN, ENTER GAME, LATENCY, CHAT */
	SPtr_SendPktBuf CPkt_LogIn();
	SPtr_SendPktBuf CPkt_EnterGame(uint32_t player_id);
	SPtr_SendPktBuf CPkt_Chat(UINT32 sessionID, std::string msg);
	SPtr_SendPktBuf CPkt_NetworkLatency(long long timestamp);

	/* PLAYER */
	SPtr_SendPktBuf CPkt_NewPlayer();
	SPtr_SendPktBuf CPkt_RemovePlayer(uint32_t removeSessionID);
	SPtr_SendPktBuf CPkt_Player_Transform(Vec3 Pos, Vec3 Rot, int32_t movestate, Vec3 movedir, float velocity, Vec3 SpineLookDir, long long latency, float animparam_h, float animparam_v);
	SPtr_SendPktBuf CPkt_Player_Animation(int anim_upper_idx, int anim_lower_idx, float anim_param_h, float anim_param_v);
	SPtr_SendPktBuf CPkt_Player_Weapon(FBProtocol::WEAPON_TYPE weaponType);
	
	/* MONSTER */
	SPtr_SendPktBuf CPkt_NewMonster();
	SPtr_SendPktBuf CPkt_RemoveMonster(uint32_t monsterID);
	SPtr_SendPktBuf CPkt_Monster_Transform(uint32_t monsterID, Vec3 Pos, Vec3 Rot);
	SPtr_SendPktBuf CPkt_Monster_HP(uint32_t monsterID, float hp);
	SPtr_SendPktBuf CPkt_Monster_State(uint32_t monsterID, FBProtocol::MONSTER_STATE_TYPE state);
	
	/* BULLET */
	SPtr_SendPktBuf CPkt_Bullet_OnShoot(uint32_t playerID, uint32_t gunID, uint32_t bulletID, Vec3 ray);
	SPtr_SendPktBuf CPkt_Bullet_OnCollision(uint32_t playerID, uint32_t gunID, uint32_t bulletID);


private:

	/// +------------------------
	///	         UTILITY 
	/// ------------------------+
	static GamePlayerInfo GetPlayerInfo(const FBProtocol::Player* player);
	static Vec3 GetVector3(const FBProtocol::Vector3* vec3);
	static Vec4 GetVector4(const FBProtocol::Vector4* vec4);

	static Vec3 CalculateDirection(float yAngleRadian);
	static Vec3 lerp(Vec3 CurrPos, Vec3 TargetPos, float PosLerpParam);
};

