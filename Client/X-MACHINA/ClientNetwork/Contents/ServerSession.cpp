#include "stdafx.h"
#include "ServerSession.h"

#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Include/Protocol/FBProtocol_generated.h"

ServerSession::ServerSession()
{
}

ServerSession::~ServerSession()
{
}

void ServerSession::OnConnected()
{
	// ID, Password �Է�
	std::string ID;
	std::string Password;

	LOG_MGR->SetColor(TextColor::BrightWhite);
	LOG_MGR->Cout("ID : ");
	std::cin >> ID;
	LOG_MGR->Cout("Password : ");
	std::cin >> Password;
	LOG_MGR->SetColor(TextColor::Default);


	/* SEND LOGIN PACKET */
	auto CPktBuf = FBS_FACTORY->CPkt_LogIn(ID, Password);
	Send(CPktBuf);

}

void ServerSession::OnDisconnected()
{
	//cout << "Disconnected" << endl;
}

void ServerSession::OnSend(UINT32 len)
{
	int i = 0;

}

UINT32 ServerSession::OnRecv(BYTE* buffer, UINT32 len)
{
	// ��Ŷ �ؼ� 
	//std::cout << this->GetID() << "RECV : " << static_cast<void*>(buffer) << "  : Bytes-" << len << std::endl;

	UINT32 ProcessDataSize = mRemainDataSize;
	/* ���ļ� ���� ��Ŷ���� ó���Ѵ�. */
	while (ProcessDataSize < len) {

		UINT32 RemainSize = len - ProcessDataSize;
		if (RemainSize < sizeof(PacketHeader)) {
			mRemainDataSize = RemainSize;
			break;
		}
		PacketHeader* packet = reinterpret_cast<PacketHeader*>(buffer + ProcessDataSize);
		if (RemainSize < packet->PacketSize) {
			mRemainDataSize = RemainSize;
			break;
		}

		/* ��Ŷ �ؼ� */
		FBsPacketFactory::ProcessFBsPacket(static_pointer_cast<Session>(shared_from_this()), buffer + ProcessDataSize, packet->PacketSize);
		ProcessDataSize += packet->PacketSize;

		if (packet->PacketSize == 0) {
			// assert(0) 
			break;
		}
	}

	return len;
}

