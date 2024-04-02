#include "stdafx.h"
#include "ServerFBsPktFactory.h"
#include "Enum_generated.h"
#include "FBProtocol_generated.h"
#include "Struct_generated.h"
#include "Transform_generated.h"
#include "../PacketFactory.h"
FlatPacketHandlerFunc GFlatPacketHandler[UINT16_MAX]{};


bool ProcessFBsPkt_Invalid(SPtr_PacketSession& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	// TODO : Log
	return false;
}

bool ProcessFBsPkt_SPkt_LogIn(SPtr_PacketSession& session, const FBProtocol::SPkt_LogIn& pkt)
{
	if (pkt.success() == false)
		return true;

	std::string PlayerName = pkt.players()->Get(1)->name()->str();
	//LOG_MGR->Cout(PlayerName, "\n");

	if (pkt.players()->size() == 0) {
		// ĳ���� ����â
	}

	// ���� UI ��ư ������ ���� ����
	uint64_t playerIdx = 0; /* �ӽ� */
	auto CPktBuf       = PacketFactory::CreateSendBuffer_CPkt_CEnterGame(playerIdx);
	session->Send(CPktBuf);

	return true;
}

bool ProcessFBsPkt_SPkt_EnterGame(SPtr_PacketSession& session, const FBProtocol::SPkt_EnterGame& pkt)
{
	return true;
}

bool ProcessFBsPkt_SPkt_Chat(SPtr_PacketSession& session, const FBProtocol::SPkt_Chat& pkt)
{
	std::cout << "--->[RECV]" << pkt.message()->c_str() << std::endl;
	return true;
}

bool ProcessFBsPkt_SPkt_Transform(SPtr_PacketSession& session, const FBProtocol::SPkt_Transform& pkt)
{
	// ��Ŷ���� ��ü ID�� ��ġ ���� ��������
	int objID = pkt.object_id();
	float x   = 0.0f, y = 0.0f, z = 0.0f;

	// ��Ŷ���� ��ġ ������ �ִ��� Ȯ���ϰ� ������
	if (pkt.trans() && pkt.trans()->position()) {
		x = pkt.trans()->position()->x();
		y = pkt.trans()->position()->y();
		z = pkt.trans()->position()->z();
	}
	else {
		std::cerr << "Error: Missing position information in transform packet!" << std::endl;
		return false;
	}

	// ���� ���
	std::cout << "Object ID: " << objID << " (x: " << x << ", y: " << y << ", z: " << z << ")" << std::endl;
	return true;
}

bool ProcessFBsPkt_SPkt_KeyInput(SPtr_PacketSession& session, const FBProtocol::SPkt_KeyInput& pkt)
{
	return false;
}
