#include "stdafx.h"
#include "ServerSession.h"

#include "Script_LoginUI.h"

#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Include/Protocol/FBProtocol_generated.h"

ServerSession::ServerSession()
{
}

ServerSession::~ServerSession()
{
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
	// 현재까지 처리된 데이터 크기
	UINT32 ProcessDataSize = 0;

	// 기존 남은 데이터를 고려하여 전체 크기를 계산
	UINT32 TotalSize = len + mRemainDataSize;

	while (ProcessDataSize < TotalSize) {
		UINT32 RemainSize = TotalSize - ProcessDataSize;

		// 남은 데이터가 PacketHeader의 크기보다 작으면 다음 번 수신에서 처리
		if (RemainSize < sizeof(PacketHeader)) {
			mRemainDataSize = RemainSize; // 남은 데이터 크기 저장 
			break;
		}

		// 패킷 헤더를 파싱 
		PacketHeader* packet = reinterpret_cast<PacketHeader*>(buffer + ProcessDataSize);

		// 남은 데이터가 패킷 전체 크기보다 적으면 다음 번 수신에서 처리 
		if (RemainSize < packet->PacketSize) { 
			mRemainDataSize = RemainSize; // 남은 데이터 크기 저장 
			break;
		}

		// 패킷 처리
		FBsPacketFactory::ProcessFBsPacket(static_pointer_cast<Session>(shared_from_this()), buffer + ProcessDataSize, packet->PacketSize);

		// 처리된 데이터 크기만큼 증가 
		ProcessDataSize += packet->PacketSize;
	}

	// 남은 데이터 크기 갱신 
	mRemainDataSize = TotalSize - ProcessDataSize;

	return len;
}

