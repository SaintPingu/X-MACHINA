/// +------------------------
///		Protocol.fbs
/// ------------------------+

/// 기존 스키마는 그대로 유지되며, 여기에는 클래스에 추가할 함수들을 정의하겠습니다.

#pragma once
#undef max
#include <flatbuffers/flatbuffers.h>
#include "ClientNetwork/Include/Protocol/FBProtocol_generated.h"
#include "ClientNetwork/Include/Protocol/Enum_generated.h"
#include "ClientNetwork/Include/Protocol/Transform_generated.h"
#include "ClientNetwork/Include/Protocol/Struct_generated.h"

#include "NetworkEvents.h"
#include "GamePlayer.h"
#include "GameMonster.h"

#include "InputMgr.h"

namespace PLAYER_MOVE_STATE
{
	constexpr int32_t Start = 0;
	constexpr int32_t Progress = 1;
	constexpr int32_t End = 2;
	constexpr int32_t Default = 3; // 키 입력이 아예 없을 때는 Default 
}

#define FBS_FACTORY FBsPacketFactory::GetInst()

class FBsPacketFactory
{
	DECLARE_SINGLETON(FBsPacketFactory);

public:
	static std::atomic<long long>	CurrLatency;

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
	static bool Process_SPkt_Player_OnSkill(SPtr_Session session, const FBProtocol::SPkt_PlayerOnSkill& pkt); // 추가
	static bool Process_SPkt_Player_AimRotation(SPtr_Session session, const FBProtocol::SPkt_Player_AimRotation& pkt);
	static bool Process_SPkt_Player_State(SPtr_Session session, const FBProtocol::SPkt_Player_State& pkt);


	/* MONSTER */
	static bool Process_SPkt_NewMonster(SPtr_Session session, const FBProtocol::SPkt_NewMonster& pkt);
	static bool Process_SPkt_RemoveMonster(SPtr_Session session, const FBProtocol::SPkt_RemoveMonster& pkt);
	static bool Process_SPkt_DeadMonster(SPtr_Session session, const FBProtocol::SPkt_DeadMonster& pkt); // 추가
	static bool Process_SPkt_Monster_Transform(SPtr_Session session, const FBProtocol::SPkt_Monster_Transform& pkt);
	static bool Process_SPkt_Monster_HP(SPtr_Session session, const FBProtocol::SPkt_Monster_HP& pkt);
	static bool Process_SPkt_Monster_State(SPtr_Session session, const FBProtocol::SPkt_Monster_State& pkt);
	static bool Process_SPkt_Monster_Target(SPtr_Session session, const FBProtocol::SPkt_MonsterTarget& pkt);

	/* PHERO */
	static bool Process_SPkt_GetPhero(SPtr_Session session, const FBProtocol::SPkt_GetPhero& pkt);

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
	SPtr_SendPktBuf CPkt_Player_AimRotation(float aim_rotation_y, float spine_angle);
	SPtr_SendPktBuf CPkt_Player_Weapon(WeaponName weaponName);
	SPtr_SendPktBuf CPkt_Player_OnSkill(FBProtocol::PLAYER_SKILL_TYPE skillType, int mindcontrol_monster_id = -1); // 추가

	/* MONSTER */
	SPtr_SendPktBuf CPkt_NewMonster(uint32_t monster_id, FBProtocol::MONSTER_TYPE montser_type);
	SPtr_SendPktBuf CPkt_RemoveMonster(uint32_t monsterID);
	SPtr_SendPktBuf CPkt_DeadMonster(uint32_t monsterID, Vec2 deadPoint);

	SPtr_SendPktBuf CPkt_Monster_Transform(uint32_t monsterID, Vec3 Pos, Vec3 Rot);
	SPtr_SendPktBuf CPkt_Monster_HP(uint32_t monsterID, float hp);
	SPtr_SendPktBuf CPkt_Monster_State(uint32_t monsterID, FBProtocol::MONSTER_BT_TYPE state);


	/* PHERO */
	SPtr_SendPktBuf CPkt_GetPhero(uint32_t phero_id, uint32_t player_id);

	/* BULLET */
	SPtr_SendPktBuf CPkt_Bullet_OnShoot(Vec3 ray);
	SPtr_SendPktBuf CPkt_Bullet_OnCollision(uint32_t playerID, uint32_t gunID, uint32_t bulletID);

private:
	/// +------------------------
	///	         UTILITY 
	/// ------------------------+
	static GamePlayerInfo GetPlayerInfo(const FBProtocol::Player* player);
	static GameMonsterInfo GetMonsterInfo(const FBProtocol::Monster* monster);

	static Vec3 GetVector3(const FBProtocol::Vector3* vec3);
	static Vec4 GetVector4(const FBProtocol::Vector4* vec4);
	static Vec3 GetPosition_Vec2(const FBProtocol::Position_Vec2* vec2);
	static Vec4 GetRot_y(const float rot_y);


	static Vec3 CalculateDirection(float yAngleRadian);
	static Vec3 lerp(Vec3 CurrPos, Vec3 TargetPos, float PosLerpParam);
};

