#include "pch.h"
#include "NetworkInterface.h"
#include "NetworkManager.h"
#include "SessionController.h"

#include "ServerNetwork.h"
#include "Lock.h"


NetworkInterface::NetworkInterface()
{
	mSessionController = std::make_shared<SessionController>();
}

NetworkInterface::~NetworkInterface()
{
	if (mIocpHandle) {
		::CloseHandle(mIocpHandle);
	}
	mSessionController = nullptr;
}

bool NetworkInterface::RegisterIocp(SPtr_NetObj netObj)
{
	// Iocp Handle 을 통해서 해당 소켓을 관찰하도록 등록한다.
	SOCKET sock = netObj->GetSocketData().GetSocket();
	HANDLE handle = netObj->GetSocketHandle();
	if (handle) {
		return ::CreateIoCompletionPort(handle
									, mIocpHandle
									, 0
									, 0);
	}
	return false;
}

bool NetworkInterface::Dispatch_CompletedTasks_FromIOCP(UINT32 msTimeOut)
{
	/// +=========================== IOCP Queue ============================
	///  (완료된 일감들...)
	/// ____________________________________________________________________
	/// (+_+) 감시한다...
	/// (timeoutMs : INFINITE -> 무한대기 (블로킹) / 0 -> (논블로킹)
	/// +==================================================================+

	DWORD				BytesTransferred = 0;
	ULONG_PTR			key              = 0;
	OverlappedObject*	overObj          = nullptr;

	LPOVERLAPPED lpOverlapped = nullptr;
	bool IsTaskExisted = ::GetQueuedCompletionStatus(mIocpHandle
												, OUT &BytesTransferred
												, OUT &key
												, OUT &lpOverlapped/*reinterpret_cast<LPOVERLAPPED*>(&overObj)*/
												, msTimeOut);
	if (TRUE == IsTaskExisted) {
		/// +------------
		///    SUCCESS 
		/// ------------+
		overObj = reinterpret_cast<OverlappedObject*>(lpOverlapped);
		SPtr_NetObj netObj = overObj->GetOwner();
		netObj->Dispatch(overObj, BytesTransferred);

	}
	else
	{
		overObj = reinterpret_cast<OverlappedObject*>(lpOverlapped);

		/// +------------
		///     FAIL
		/// ------------+
		INT32 errCode = ::WSAGetLastError();
		// TODO : Log
		//std::cout << "Handle Error : " << errCode << std::endl;

		switch (errCode)
		{
		case WAIT_TIMEOUT:
			return false;
		default:
			/* errCode : 64 = Disconnect ( Recv 0 ) */
			SPtr_NetObj netObj = overObj->GetOwner();
			netObj->Dispatch(overObj, BytesTransferred);
			break;
		}

	}


	return true;
}

bool NetworkInterface::Start(std::wstring ip, UINT16 portNum)
{
	/* Sock Addr */
	::memset(&mSockAddr, 0, sizeof(mSockAddr));
	mSockAddr.sin_family = AF_INET;
	mSockAddr.sin_addr   = NETWORK_MGR->Ip2Address(ip.c_str());
	mSockAddr.sin_port   = ::htons(portNum); // 포트 번호 : 16비트 숫자이다. host to Network - short(16bit) ( htons )

	/* IO Completion Port - Handle */
	mIocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE
											, 0
											, 0
											, 0);


	return mIocpHandle != INVALID_HANDLE_VALUE;
}

void NetworkInterface::Close()
{
}


void NetworkInterface::Send(UINT32 sessionID, SPtr_SendPktBuf sendBuf)
{
	mSessionController->Send(sessionID, sendBuf); // READ Lock

}

void NetworkInterface::Broadcast(SPtr_SendPktBuf sendBuf)
{
	mSessionController->Broadcast(sendBuf); // READ Lock

}


SPtr_Session NetworkInterface::CreateSession()
{

	SPtr_Session session = mSessionController->CreateSession(shared_from_this());
	
	if(RegisterIocp(session) == false)
		return nullptr;
	return session;
}

void NetworkInterface::AddSession(UINT32 sessionID, SPtr_Session session)
{
	mSessionController->AddSession(sessionID, session); // WRITE lock 
}

void NetworkInterface::DeleteSession(UINT32 sessionID)
{
	mSessionController->ReleaseSession(sessionID); // WRITE lock
}

