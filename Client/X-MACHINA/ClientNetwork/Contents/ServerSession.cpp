#include "stdafx.h"
#include "ServerSession.h"

ServerSession::ServerSession()
{
}

ServerSession::~ServerSession()
{
}

void ServerSession::OnConnected()
{
	// 입장 UI 버튼 눌러서 게임 입장
	//flatbuffers::FlatBufferBuilder builder;
	//bool IsSuccess = true;
	//auto CPktBuf = PacketFactory::CreateSendBuffer_CPkt_LogIn(IsSuccess);
	//Send(CPktBuf);

	//auto pkt = FBProtocol::CreateCPkt_LogIn(builder);
	//builder.Finish(pkt);

	//const uint8_t* bufferPointer = builder.GetBufferPointer();
	//const uint16_t SerializeddataSize = static_cast<uint16_t>(builder.GetSize());;


	//auto sendbuffer = ServerFBsPktFactory::MakeFBsSendPktBuf(bufferPointer, SerializeddataSize, pkt);

}
//void ServerSession::OnRecvPacket(BYTE* buffer, int32 len)
//{
//	//PacketSessionRef session = GetPacketSessionRef();
//	//PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
//
//	//// TODO : packetId 대역 체크
//	////ServerPacketHandler::HandlePacket(session, buffer, len);
//	//ServerFBsPktFactory::ProcessFBsPkt(session, buffer, len);
//
//}

//void ServerSession::OnSend(INT32 len)
//{
//	//cout << "OnSend Len = " << len << endl;
//}


void ServerSession::OnDisconnected()
{
	//cout << "Disconnected" << endl;
}

void ServerSession::OnSend(UINT32 len)
{
}

UINT32 ServerSession::OnRecv(BYTE* buffer, UINT32 len)
{
	return UINT32();
}

