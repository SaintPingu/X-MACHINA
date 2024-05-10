#include "pch.h"
#include "ClientNetwork.h"

#include "NetworkObject.h"

ClientNetwork::ClientNetwork()
{
}

ClientNetwork::~ClientNetwork()
{
}

bool ClientNetwork::Start(std::wstring ip, UINT16 portNum)
{
	NetworkInterface::Start(ip, portNum);

	for (int i = 0; i < mConnectSessionCnt; ++i) {
		SPtr_Session session = NetworkInterface::mSessionController->CreateSession(shared_from_this());
		session->SetIpPort(ip, static_cast<UINT16>(portNum));
		if(TRUE == RegisterIocp(std::static_pointer_cast<NetworkObject>(session)))
			session->RegisterIO(OverlappedIO::Type::Connect);
	}

	return true;

}

void ClientNetwork::Close()
{
	NetworkInterface::Close();

}
