#pragma once

/// +-----------------------------------------------
///					ClientNetwork 
/// ________________________________________________

/// -----------------------------------------------+

#include "NetworkInterface.h"

class ClientNetwork : public NetworkInterface
{
private:
	UINT32	mConnectSessionCnt = 1; /* 立加且 技记 荐 */

public:
	ClientNetwork();
	virtual ~ClientNetwork();

	virtual bool Start(std::wstring ip, UINT16 portNum) override;
	virtual void Close() override;

public:
	void SetConntSessionMaxCnt(UINT32 maxCnt) { mConnectSessionCnt = maxCnt; }


};

