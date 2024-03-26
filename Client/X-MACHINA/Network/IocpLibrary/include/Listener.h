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
	///				 ����
	/// ------------------------------+
	void RegisterAccept(AcceptEvent* acceptEvent); // ���ο� Session�� Accept �� �� �ֵ��� CompletionIO �� ����Ѵ�. ( �񵿱�I/O ���� ���� )
	void ProcessAccept(AcceptEvent* acceptEvent);
};

