#pragma once
#include "Script_Network.h"

class ServerSession : public PacketSession
{
public:
	ServerSession();
	~ServerSession();

	virtual void OnConnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;
	virtual void OnDisconnected() override;

};