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
	{%- for pkt in parser.total_pkt %}
	{{pkt.name}} = {{pkt.id}},
	{%- endfor %}
	// ----------+
};



bool ProcessFBsPkt_Invalid(SPtr_PacketSession& session, BYTE* buffer, int32 len);

// +---------------------------------
// Process Recv Packet - FlatBuffers
{%- for pkt in parser.recv_pkt %}
bool ProcessFBsPkt_{{pkt.name}}(SPtr_PacketSession& session, const FBProtocol::{{pkt.name}}&pkt);
{%- endfor %}
// ---------------------------------+

class {{output}}
{


public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GFlatPacketHandler[i] = ProcessFBsPkt_Invalid;

		// +---------------------------------
		// Process Recv Packet - FlatBuffers
		{%- for pkt in parser.recv_pkt %}
		GFlatPacketHandler[static_cast<uint16>(FBPkt_ProtocolID::{{pkt.name}})] = [](SPtr_PacketSession& session, BYTE* buffer, int32 len) { return ProcessFBsPkt<FBProtocol::{{pkt.name}}>(ProcessFBsPkt_{{pkt.name}}, session, buffer, len); };
		{%- endfor %}
		// ---------------------------------+
	}

	static bool ProcessFBsPkt(SPtr_PacketSession & session, BYTE * buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GFlatPacketHandler[header->id](session, buffer, len);
	}

	// +------------------------------
	// Make Send Packet - FlatBuffers
	{%- for pkt in parser.send_pkt %}
	static SPtr_SendPktBuf MakeFBsSendPktBuf(const uint8_t* bufferPointer, const uint16 SerializedDataSize, const flatbuffers::Offset<FBProtocol::{{pkt.name}}>&pkt) { return MakeFBsSendPktBuf(bufferPointer, SerializedDataSize, pkt, static_cast<uint16>(FBPkt_ProtocolID::{{pkt.name}})); }
	{%- endfor %}
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
