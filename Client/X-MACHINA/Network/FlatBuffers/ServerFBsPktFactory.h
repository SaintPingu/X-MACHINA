#pragma once
#undef max 
#include <flatbuffers/flatbuffers.h>
#include "FBProtocol_generated.h"

using FlatPacketHandlerFunc = std::function<bool(SPtr_PacketSession&, BYTE*, int32)>;
extern FlatPacketHandlerFunc GFlatPacketHandler[UINT16_MAX];


enum class FBPkt_ProtocolID : uint16 // Server ----> Client
{
	// +----------
	// ENUM CLASS
	CPkt_LogIn = 1000,
	SPkt_LogIn = 1001,
	CPkt_EnterGame = 1002,
	SPkt_EnterGame = 1003,
	CPkt_Chat = 1004,
	SPkt_Chat = 1005,
	CPkt_Transform = 1006,
	SPkt_Transform = 1007,
	CPkt_KeyInput = 1008,
	SPkt_KeyInput = 1009,
	// ----------+
};



bool ProcessFBsPkt_Invalid(SPtr_PacketSession& session, BYTE* buffer, int32 len);

// +---------------------------------
// Process Recv Packet - FlatBuffers
bool ProcessFBsPkt_SPkt_LogIn(SPtr_PacketSession& session, const FBProtocol::SPkt_LogIn&pkt);
bool ProcessFBsPkt_SPkt_EnterGame(SPtr_PacketSession& session, const FBProtocol::SPkt_EnterGame&pkt);
bool ProcessFBsPkt_SPkt_Chat(SPtr_PacketSession& session, const FBProtocol::SPkt_Chat&pkt);
bool ProcessFBsPkt_SPkt_Transform(SPtr_PacketSession& session, const FBProtocol::SPkt_Transform&pkt);
bool ProcessFBsPkt_SPkt_KeyInput(SPtr_PacketSession& session, const FBProtocol::SPkt_KeyInput&pkt);
// ---------------------------------+

class ServerFBsPktFactory
{


public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GFlatPacketHandler[i] = ProcessFBsPkt_Invalid;

		// +---------------------------------
		// Process Recv Packet - FlatBuffers
		GFlatPacketHandler[static_cast<uint16>(FBPkt_ProtocolID::SPkt_LogIn)] = [](SPtr_PacketSession& session, BYTE* buffer, int32 len) { return ProcessFBsPkt<FBProtocol::SPkt_LogIn>(ProcessFBsPkt_SPkt_LogIn, session, buffer, len); };
		GFlatPacketHandler[static_cast<uint16>(FBPkt_ProtocolID::SPkt_EnterGame)] = [](SPtr_PacketSession& session, BYTE* buffer, int32 len) { return ProcessFBsPkt<FBProtocol::SPkt_EnterGame>(ProcessFBsPkt_SPkt_EnterGame, session, buffer, len); };
		GFlatPacketHandler[static_cast<uint16>(FBPkt_ProtocolID::SPkt_Chat)] = [](SPtr_PacketSession& session, BYTE* buffer, int32 len) { return ProcessFBsPkt<FBProtocol::SPkt_Chat>(ProcessFBsPkt_SPkt_Chat, session, buffer, len); };
		GFlatPacketHandler[static_cast<uint16>(FBPkt_ProtocolID::SPkt_Transform)] = [](SPtr_PacketSession& session, BYTE* buffer, int32 len) { return ProcessFBsPkt<FBProtocol::SPkt_Transform>(ProcessFBsPkt_SPkt_Transform, session, buffer, len); };
		GFlatPacketHandler[static_cast<uint16>(FBPkt_ProtocolID::SPkt_KeyInput)] = [](SPtr_PacketSession& session, BYTE* buffer, int32 len) { return ProcessFBsPkt<FBProtocol::SPkt_KeyInput>(ProcessFBsPkt_SPkt_KeyInput, session, buffer, len); };
		// ---------------------------------+
	}

	static bool ProcessFBsPkt(SPtr_PacketSession & session, BYTE * buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GFlatPacketHandler[header->id](session, buffer, len);
	}

	// +------------------------------
	// Make Send Packet - FlatBuffers
	static SPtr_SendPktBuf MakeFBsSendPktBuf(const uint8_t* bufferPointer, const uint16 SerializedDataSize, const flatbuffers::Offset<FBProtocol::CPkt_LogIn>&pkt) { return MakeFBsSendPktBuf(bufferPointer, SerializedDataSize, pkt, static_cast<uint16>(FBPkt_ProtocolID::CPkt_LogIn)); }
	static SPtr_SendPktBuf MakeFBsSendPktBuf(const uint8_t* bufferPointer, const uint16 SerializedDataSize, const flatbuffers::Offset<FBProtocol::CPkt_EnterGame>&pkt) { return MakeFBsSendPktBuf(bufferPointer, SerializedDataSize, pkt, static_cast<uint16>(FBPkt_ProtocolID::CPkt_EnterGame)); }
	static SPtr_SendPktBuf MakeFBsSendPktBuf(const uint8_t* bufferPointer, const uint16 SerializedDataSize, const flatbuffers::Offset<FBProtocol::CPkt_Chat>&pkt) { return MakeFBsSendPktBuf(bufferPointer, SerializedDataSize, pkt, static_cast<uint16>(FBPkt_ProtocolID::CPkt_Chat)); }
	static SPtr_SendPktBuf MakeFBsSendPktBuf(const uint8_t* bufferPointer, const uint16 SerializedDataSize, const flatbuffers::Offset<FBProtocol::CPkt_Transform>&pkt) { return MakeFBsSendPktBuf(bufferPointer, SerializedDataSize, pkt, static_cast<uint16>(FBPkt_ProtocolID::CPkt_Transform)); }
	static SPtr_SendPktBuf MakeFBsSendPktBuf(const uint8_t* bufferPointer, const uint16 SerializedDataSize, const flatbuffers::Offset<FBProtocol::CPkt_KeyInput>&pkt) { return MakeFBsSendPktBuf(bufferPointer, SerializedDataSize, pkt, static_cast<uint16>(FBPkt_ProtocolID::CPkt_KeyInput)); }
	// ------------------------------+

private:
	template<typename PacketType, typename ProcessFunc>
	static bool ProcessFBsPkt(ProcessFunc func, SPtr_PacketSession& session, BYTE* buffer, int32 len)
	{
		// Deserialize the packet
	    const PacketType* Packet = flatbuffers::GetRoot<PacketType>(buffer + sizeof(PacketHeader));

		// Check if deserialization was successful
		if (!Packet) {
			return false; // Failed to deserialize packet
		}

		// Call the processing function
		return func(session, *Packet);
	}

	template<typename T>
	static SPtr_SendPktBuf MakeFBsSendPktBuf(const uint8_t* bufferPointer, const uint16 SerializedDataSize, const flatbuffers::Offset<T>& flatBufData, uint16 ProtocolId)
	{
		/// +-----------------------------------------------------------------
		/// [PacketHeader]( 4byte ) + [Data](FlatBuffers-Finish : Serialized)
		/// -----------------------------------------------------------------+


		const uint16 packetSize         = SerializedDataSize + sizeof(PacketHeader);

		SPtr_SendPktBuf sendBuffer        = SENDPACKET_MGR->Open(packetSize);
		PacketHeader* header            = reinterpret_cast<PacketHeader*>(sendBuffer->GetBuffer());
		header->size                    = packetSize;
		header->id                      = ProtocolId;

		std::memcpy(&header[1], bufferPointer, SerializedDataSize);

		sendBuffer->Close(packetSize);

		return sendBuffer;
	}

};