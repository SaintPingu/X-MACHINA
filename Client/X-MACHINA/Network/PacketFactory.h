#pragma once
class PacketFactory
{
public:
	static SPtr_SendPktBuf CreateSendBuffer_CPkt_Chat(const std::string msg);
	static SPtr_SendPktBuf CreateSendBuffer_CPkt_CEnterGame(uint64_t playerIdx);
	static SPtr_SendPktBuf CreateSendBuffer_CPkt_Transform( Vec3 Pos, Vec3 Rot, Vec3 Scale);
	static SPtr_SendPktBuf CreateSendBuffer_CPkt_LogIn(bool& success);

};

