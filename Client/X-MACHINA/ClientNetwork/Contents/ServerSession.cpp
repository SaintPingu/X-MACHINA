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
	// ������� ó���� ������ ũ��
	UINT32 ProcessDataSize = 0;

	// ���� ���� �����͸� ����Ͽ� ��ü ũ�⸦ ���
	UINT32 TotalSize = len + mRemainDataSize;

	while (ProcessDataSize < TotalSize) {
		UINT32 RemainSize = TotalSize - ProcessDataSize;

		// ���� �����Ͱ� PacketHeader�� ũ�⺸�� ������ ���� �� ���ſ��� ó��
		if (RemainSize < sizeof(PacketHeader)) {
			mRemainDataSize = RemainSize; // ���� ������ ũ�� ���� 
			break;
		}

		// ��Ŷ ����� �Ľ� 
		PacketHeader* packet = reinterpret_cast<PacketHeader*>(buffer + ProcessDataSize);

		// ���� �����Ͱ� ��Ŷ ��ü ũ�⺸�� ������ ���� �� ���ſ��� ó�� 
		if (RemainSize < packet->PacketSize) { 
			mRemainDataSize = RemainSize; // ���� ������ ũ�� ���� 
			break;
		}

		// ��Ŷ ó��
		FBsPacketFactory::ProcessFBsPacket(static_pointer_cast<Session>(shared_from_this()), buffer + ProcessDataSize, packet->PacketSize);

		// ó���� ������ ũ�⸸ŭ ���� 
		ProcessDataSize += packet->PacketSize;
	}

	// ���� ������ ũ�� ���� 
	mRemainDataSize = TotalSize - ProcessDataSize;

	return len;
}

