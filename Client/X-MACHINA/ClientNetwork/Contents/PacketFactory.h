#pragma once
#include "InputMgr.h"

/* 키 관련 코드 */


class PacketFactory
{

public:
	static SPtr_SendPktBuf CreateSendBuffer_CPkt_Chat(const std::string msg);
	static SPtr_SendPktBuf CreateSendBuffer_CPkt_CEnterGame(uint64_t playerIdx);
	static SPtr_SendPktBuf CreateSendBuffer_CPkt_Transform( Vec3 Pos, Vec3 Rot, Vec3 Scale , Vec3 SpineLookDir, long long timestamp);
	static SPtr_SendPktBuf CreateSendBuffer_CPkt_LogIn(bool& success);
	static SPtr_SendPktBuf CreateSendBuffer_CPkt_KeyInput(    GameKeyInfo::KEY		 key
															, GameKeyInfo::KEY_STATE KeyState
															, GameKeyInfo::MoveKey	 moveKey
															,              Vec2		 mouseDelta);

};

