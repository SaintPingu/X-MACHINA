#pragma once
#include "Iocp.h"
#include "NetAddress.h"

class AcceptEvent;
class ServerService;

/// +------------------------------
///			    LISTENER 
/// ------------------------------+
class Listener : public IocpObject
{

protected:
	SOCKET					mSocket = INVALID_SOCKET;
	NetVector<AcceptEvent*>	mAcceptEvents;
	ServerServiceRef		mService;



public:
	Listener() = default;
	~Listener();

public:
	/// +------------------------------
	///			    SOCKET 
	/// ------------------------------+
	bool StartAccept(ServerServiceRef service);
	void CloseSocket();

public:
	/// +------------------------------
	///			   INTERFACE
	/// ------------------------------+
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
	/// +------------------------------
	///				 수신
	/// ------------------------------+
	void RegisterAccept(AcceptEvent* acceptEvent); // 새로운 Session을 Accept 할 수 있도록 CompletionIO 에 등록한다. ( 비동기I/O 연결 수락 )
	void ProcessAccept(AcceptEvent* acceptEvent);
};

