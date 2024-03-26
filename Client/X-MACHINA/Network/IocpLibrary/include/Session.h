#pragma once
#include "Iocp.h"
#include "IocpEvent.h"
#include "NetAddress.h"
#include "RecvBuffer.h"

class Service;
enum class BufferInfo {
	BufferSize = 0x10'000, // 64kb
};
/*--------------
	Session
---------------*/

class Session : public IocpObject
{
private:
	USE_LOCK;

private:
	/* IocpEvent ���� */
	ConnectEvent			mConnectEvent    = {};
	DisconnectEvent			mDisconnectEvent = {};
	RecvEvent				mRecvEvent       = {};
	SendEvent				mSendEvent       = {};

private:
	std::weak_ptr<Service>	mService     = {};
	SOCKET					mSocket      = INVALID_SOCKET;
	NetAddress				mNetAddress  = {};
	Atomic<bool>			mIsConnected = false;
	/* ���� ���� */
	RecvBuffer				mRecvBuffer  = {};

	/* �۽� ���� */
	NetQueue<SPtr_SendPktBuf>	mSendQueue        = {};
	Atomic<bool>			mIsSendRegistered = false;




public:
	Session();
	virtual ~Session();

public:
	/// +------------------------------
	///			    NETWORK 
	/// ------------------------------+
	void				Send(SPtr_SendPktBuf sendBuffer);
	bool				Connect();
	void				Disconnect(const WCHAR* cause);




	void				SetService(std::shared_ptr<Service> service) { mService = service; }

public:
						/* ���� ���� */
	std::shared_ptr<Service>	GetService() { return mService.lock(); }
	void				SetNetAddress(NetAddress address)	{ mNetAddress = address; }
	NetAddress			GetNetAddress()						{ return mNetAddress; }
	SOCKET				GetSocket()							{ return mSocket; }
	SessionRef			GetSession()						{  return std::static_pointer_cast<Session>(shared_from_this()); }
	RecvBuffer&			GetRecvBuffer()						{ return mRecvBuffer; }

	bool				IsConnected() { return mIsConnected; }
	SessionRef			GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }

private:
						/* �������̽� ���� */
	virtual HANDLE		GetHandle() override;
	virtual void		Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

public:
	/// +------------------------------
	///			   REGISTER
	/// ------------------------------+
	bool				RegisterConnect();
	bool				RegisterDisconnect();
	void				RegisterRecv();
	void				RegisterSend();

	/// +------------------------------
	///			   PROCESS
	/// ------------------------------+
	void				ProcessConnect();
	void				ProcessDisconnect();
	void				ProcessRecv(int32 numOfBytes);
	void				ProcessSend(int32 numOfBytes);

	void				HandleError(int32 errorCode); // Show Error Code  

protected:

	// ������ ��ο��� �����ε� 
	virtual void		OnConnected() { }
	virtual int32		OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void		OnSend(int32 len) { }
	virtual void		OnDisconnected() { }




};

