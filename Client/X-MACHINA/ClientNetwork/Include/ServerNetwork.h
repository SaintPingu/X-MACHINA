#pragma once

/// +-----------------------------------------------
///					   ServerNetwork 
/// ________________________________________________

/// -----------------------------------------------+

#include "NetworkInterface.h"
#include "Listener.h"

class ServerNetwork : public NetworkInterface
{

private:
	SPtr_Listener mListener = {};

public:
	ServerNetwork();
	virtual ~ServerNetwork();

	/* DECREASE REFERENCE */
	void DecRef_Listener()			{ mListener = nullptr; }
	void DecRef_AccpetOverlapped()	{ mListener->DecRef_Accepts(); }

public:
	/* START / CLOSE */
	virtual bool Start(std::wstring ip, UINT16 portNum) override;
	virtual void Close() override;
	
public:
	/* GETTER / SETTER */
	SPtr_Listener GetListener() { return mListener; }
	void SetSockAddrIn(SOCKADDR_IN sockaddr) { mListener->GetSocketData().SetSockAddrIn(sockaddr);}

	

};

