#pragma once

/// +-----------------------------------------------
///					   Session 
/// ________________________________________________
///	구조
///					
/// ----------------------------------------------+


#include "SocketData.h"
#include "OverlappedObject.h"
#include "NetworkObject.h"
#include "PacketRecvBuf.h"
#include "PacketSendBuf.h"
#include "Lock.h"


struct OverLapped {
	Overlapped_Connect			Connect = {};
	Overlapped_DisConnect		Disconnect = {};
	Overlapped_Recv				Recv = {};
	Overlapped_Send				Send = {};
};

struct PacketBuffer {
	/* Send */
	//concurrency::concurrent_queue<SPtr_SendPktBuf> SendPkt_Queue;

	std::queue<SPtr_SendPktBuf> SendPkt_Queue = {}; /* Scatter-Gather를 위해서 Queue에 저장 */
	std::atomic<bool>			IsSendRegistered = false;

	/* Recv */
	PacketRecvBuf* RecvPkt = {};
};


class Session : public NetworkObject
{
private:
	USE_LOCK;

	std::atomic_int LockWrite_ThreadID = -1;
	std::atomic_bool LockWrite_bool = false;

	std::weak_ptr<class NetworkInterface> mOwnerNI = {};		/* Server Network or Client Network - Set Owner */
	std::atomic<bool>					  mIsConnected = false;	/* Check If Client Connect to Server */
	OverLapped							  mOverlapped = {};		/* Overlapped I/O Object */
	PacketBuffer						  mPacketBuffer = {};		/* send / recv Packet Buffer */
protected:
	/* Session class을 상속받은 class는 신호를 받는다. */
	virtual void	OnConnected() {};
	virtual void	OnDisconnected() {};
	virtual void	OnSend(UINT32 len) {};
	virtual UINT32	OnRecv(BYTE* buf, UINT32 len) { return len; }

public:
	Session();
	virtual ~Session();
	virtual void Dispatch(class OverlappedObject* overlapped, UINT32 bytes = 0) override;

public:
	/* I/O - Register → Overlapped Complete → Process */
	void RegisterIO(OverlappedIO::Type IoType);
	void ProcessIO(OverlappedIO::Type IoType, INT32 BytesTransferred = 0);

	/* Error Code 에 따른 Session 처리 */
	void ProcessError(INT32 errCode);

	/* Network */
	void Send(SPtr_SendPktBuf buf);
	void Connect();
	void Disconnect(const WCHAR* cause);

public:
	/* Set */
	void SetOwerNetworkInterface(SPtr_NI networkInterface) { mOwnerNI = networkInterface; }
	void SetIpPort(std::wstring ip, UINT32 port);

	/* Get */
	std::shared_ptr<NetworkInterface> GetOwnerNI() { return mOwnerNI.lock(); }
	PacketRecvBuf& GetRecvPktBuf() { return *mPacketBuffer.RecvPkt; }
	bool IsConnected() { return mIsConnected.load(); }

};

