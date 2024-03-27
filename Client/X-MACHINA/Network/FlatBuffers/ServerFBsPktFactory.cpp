#include "pch.h"
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
		// 캐릭터 생성창
	}

	// 입장 UI 버튼 눌러서 게임 입장
	uint64_t playerIdx = 0; /* 임시 */
	auto CPktBuf       = PacketFactory::CreateSendBuffer_CPkt_CEnterGame(playerIdx);
	session->Send(CPktBuf);

	return true;
}

bool ProcessFBsPkt_SPkt_EnterGame(SPtr_PacketSession& session, const FBProtocol::SPkt_EnterGame& pkt)
{
	return false;
}

bool ProcessFBsPkt_SPkt_Chat(SPtr_PacketSession& session, const FBProtocol::SPkt_Chat& pkt)
{
	std::cout << "--->[RECV]" << pkt.message()->c_str() << std::endl;
	return true;
}

bool ProcessFBsPkt_SPkt_Transform(SPtr_PacketSession& session, const FBProtocol::SPkt_Transform& pkt)
{
	return false;
}

bool ProcessFBsPkt_SPkt_KeyInput(SPtr_PacketSession& session, const FBProtocol::SPkt_KeyInput& pkt)
{
	return false;
}
