#pragma once

/// +-----------------------------------------------
///					NetworkInterface 
/// ________________________________________________
///	±¸Á¶
///					NetworkInterface
///				   ¡é				¡é
///			ClientNetwork	  ServerNetwork
/// -----------------------------------------------+3

#include "SessionController.h"
class NetworkInterface : public std::enable_shared_from_this<NetworkInterface>
{
protected:
	HANDLE						mIocpHandle        = {};
	SOCKADDR_IN					mSockAddr          = {};
	SPtr_SessionController		mSessionController = {};


public:
	NetworkInterface();
	virtual ~NetworkInterface();

public:
	virtual bool Start(std::wstring ip, UINT16 portNum) ;
	virtual void Close();

	/* Register/Process - (Iocp Object/Overlapped I/O) */
	bool RegisterIocp(SPtr_NetObj netObj);
	bool Dispatch_CompletedTasks_FromIOCP(UINT32 msTimeOut);

	/* Get Variable */
	HANDLE			GetIocpHandle()		{ return mIocpHandle; }
	SOCKADDR_IN&	GetSockAddr()		{ return mSockAddr; }



	/* Session Control */
#pragma region Session Controll Functions 
public:
	SPtr_Session	CreateSession();
	void			AddSession(UINT32 sessionID, SPtr_Session session);
	void			DeleteSession(UINT32 sessionID);

	void			Send(UINT32 sessionID, SPtr_SendPktBuf sendPktBuf);
	void			Broadcast(SPtr_SendPktBuf sendBuf);

	void			SetMaxSessionCnt(UINT32 sessionLimit) { mSessionController->SetMaxSessionCnt(sessionLimit); }
	void			SetSessionConstructorFunc(std::function<SPtr_Session(void)> func) { mSessionController->SetSessionConstructorFunc(func); }
	UINT32			GetMaxSessionCnt() { return mSessionController->GetMaxSEssionCnt(); }
#pragma endregion


};

