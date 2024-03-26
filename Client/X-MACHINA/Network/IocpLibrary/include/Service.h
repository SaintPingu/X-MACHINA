#pragma once
#include "NetAddress.h"
#include "Iocp.h"
#include "Listener.h"
#include <functional>

enum class ServiceType : uint8
{
	Server, // 다른 서버와 통신 ( 서버 -- 서버 : 분산서버 )
	Client, // 클라이언트와 통신 ( 서버 -- 클라 )

};


/// +--------------------
///		  Service
/// --------------------+

using SessionFactory = std::function<SessionRef(void)>;

class Service : public std::enable_shared_from_this<Service>
{

protected:
	USE_LOCK;
	ServiceType			mType;
	NetAddress			mNetAddress = {};
	IocpRef				mIocp;

	NetSet<SessionRef>		mSessions;
	int32				mSessionsCount = 0;
	int32				mMaxSessionCount = 0;
	SessionFactory		mSessionsFactory;


public:
	Service(ServiceType type
			, NetAddress address
			, IocpRef core
			, SessionFactory factory
			, int32 maxSessionCount = 1);
	virtual ~Service();

#pragma region Start / Close 
	virtual bool		Start() abstract;
	virtual void		CloseService();
#pragma endregion

	/// +----------------------
	///  BROADCAST TO SESSIONS
	/// ----------------------+
	void				Broadcast(SPtr_SendPktBuf sendBuffer); /// 접속된 모든 클라이언트 세션에 SendBuffer 를 전송한다.

#pragma region Session Part
	SessionRef			CreateSession();
	void				AddSession(SessionRef session);
	void				ReleaseSession(SessionRef session);
#pragma endregion

#pragma region GET
public:
	bool				IsCanStart() { return mSessionsFactory != nullptr; }
	int32				GetCurrentSessionCount()	{ return mSessionsCount; }
	int32				GetMaxSessionCount()		{ return mMaxSessionCount; }
	ServiceType			GetServiceType()			{ return mType; }
	NetAddress			GetNetAddress()				{ return mNetAddress; }
	IocpRef&			GetIocp()					{ return mIocp; }
#pragma endregion


	void				SetSessionFactory(SessionFactory func) { mSessionsFactory = func; }

};

/// +--------------------------------
///			CLIENT SERVICE
/// --------------------------------+ 

class ClientService : public Service
{
public:
	ClientService(NetAddress targetAddress, IocpRef core, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~ClientService() {}

	virtual bool	Start() override;
};


/// +--------------------------------
///			SERVER SERVICE
/// --------------------------------+

class ServerService : public Service
{
public:
	ServerService(NetAddress targetAddress, IocpRef core, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~ServerService() {}

	virtual bool	Start() override;
	virtual void	CloseService() override;

private:
	ListenerRef		_listener = nullptr;
};