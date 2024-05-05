#pragma once

/// +-----------------------------------------------
///					   Session 
/// ________________________________________________
///	����
///					
/// ----------------------------------------------+


#include "SocketData.h"
#include "OverlappedObject.h"
#include "NetworkObject.h"
#include "PacketRecvBuf.h"
#include "PacketSendBuf.h"
#include "Lock.h"


struct OverLapped {
	Overlapped_Connect			Connect    = {};
	Overlapped_DisConnect		Disconnect = {};
	Overlapped_Recv				Recv       = {};
	Overlapped_Send				Send       = {};
};

struct PacketBuffer {
	/* Send */
	//concurrency::concurrent_queue<SPtr_SendPktBuf> SendPkt_Queue;

	std::queue<SPtr_SendPktBuf> SendPkt_Queue    = {}; /* Scatter-Gather�� ���ؼ� Queue�� ���� */
	std::atomic<bool>			IsSendRegistered = false;
	
	/* Recv */
	PacketRecvBuf*				RecvPkt		 	 = {};
};


class Session : public NetworkObject
{
private:
	Lock::SRWLockGuard mSRWLock{};
	//USE_LOCK;
	//std::mutex sendLock;
	//class Lock::RWLock					  mRWSendLock;

	std::weak_ptr<class NetworkInterface> mOwnerNI	      = {};		/* Server Network or Client Network - Set Owner */
	std::atomic<bool>					  mIsConnected    = false;	/* Check If Client Connect to Server */
	OverLapped							  mOverlapped     = {};		/* Overlapped I/O Object */
	PacketBuffer						  mPacketBuffer   = {};		/* send / recv Packet Buffer */
protected:
	/* Session class�� ��ӹ��� class�� ��ȣ�� �޴´�. */
	virtual void	OnConnected() {};
	virtual void	OnDisconnected() {};
	virtual void	OnSend(UINT32 len) {};
	virtual UINT32	OnRecv(BYTE* buf, UINT32 len) { return len; }

public:
	Session();
	virtual ~Session();
	virtual void Dispatch(class OverlappedObject* overlapped, UINT32 bytes = 0) override;

public:
	/* I/O - Register �� Overlapped Complete �� Process */
	void RegisterIO(OverlappedIO::Type IoType);
	void ProcessIO(OverlappedIO::Type IoType, INT32 BytesTransferred = 0);

	/* Error Code �� ���� Session ó�� */
	void ProcessError(INT32 errCode);

	/* Network */
	void Send(SPtr_SendPktBuf buf);
	void Connect();
	void Disconnect(const WCHAR* cause);

public:
	/* Set */
	void SetOwerNetworkInterface(SPtr_NI networkInterface) { mOwnerNI = networkInterface; }
	/* Get */
	std::shared_ptr<NetworkInterface> GetOwnerNI()	{ return mOwnerNI.lock(); }
	PacketRecvBuf& GetRecvPktBuf()					{ return *mPacketBuffer.RecvPkt; }
	bool IsConnected()								{ return mIsConnected.load(); }

};

