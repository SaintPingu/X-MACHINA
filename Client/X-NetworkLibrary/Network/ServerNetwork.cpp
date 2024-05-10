#include "pch.h"
#include "ServerNetwork.h"
#include "NetworkInterface.h"


ServerNetwork::ServerNetwork()
{
}

ServerNetwork::~ServerNetwork()
{
	mListener = nullptr;
}

bool ServerNetwork::Start(std::wstring ip, UINT16 portNum)
{

	NetworkInterface::Start(ip, portNum);

	mListener = std::make_shared<Listener>();

	if (FALSE == mListener->Start(ip, portNum, shared_from_this()))
		return false;
	else {
		mListener->RegisterAccept();
	}

	return true;
}

void ServerNetwork::Close()
{
	NetworkInterface::Close();
}
