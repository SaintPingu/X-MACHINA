#include "pch.h"
#include "ClientNetwork.h"

ClientNetwork::ClientNetwork(std::wstring ip, UINT32 PortNum)
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
		session->RegisterIO(OverlappedIO::Type::Connect);
	}

	return true;

}

void ClientNetwork::Close()
{
	NetworkInterface::Close();

}
