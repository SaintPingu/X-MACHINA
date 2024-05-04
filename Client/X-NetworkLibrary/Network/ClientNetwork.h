#pragma once

/// +-----------------------------------------------
///					ClientNetwork 
/// ________________________________________________

/// -----------------------------------------------+

#include "NetworkInterface.h"

class ClientNetwork : public NetworkInterface
{
private:
	UINT32	mConnectSessionCnt = 1; /* ������ ���� �� */

public:
	ClientNetwork(std::wstring ip, UINT32 PortNum);
	virtual ~ClientNetwork();

	virtual bool Start(std::wstring ip, UINT16 portNum) override;
	virtual void Close() override;

public:
	void SetConntSessionMaxCnt(UINT32 maxCnt) { mConnectSessionCnt = maxCnt; }


};

