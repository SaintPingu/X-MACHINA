#pragma once
#include "ClientNetwork/Include/Session.h"

class ServerSession : public Session
{
public:
	ServerSession();
	~ServerSession();

	virtual void	OnConnected() override;
	virtual void	OnDisconnected() override;
	virtual void	OnSend(UINT32 len) override;
	virtual UINT32	OnRecv(BYTE* buffer, UINT32 len) override;

};