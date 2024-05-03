#include "stdafx.h"
#include "PacketFactory.h"
#include "InputMgr.h"

/* FlatBuffers - Packet */
#undef max

#include <flatbuffers/flatbuffers.h>
#include "FlatBuffers/Enum_generated.h"
#include "FlatBuffers/struct_generated.h"
#include "FlatBuffers/Transform_generated.h"
#include "FlatBuffers/ServerFBsPktFactory.h"

/// +----------------------------
///			 CPkt_Chat
/// ----------------------------+
SPtr_SendPktBuf PacketFactory::CreateSendBuffer_CPkt_Chat(const std::string msg)
{
	flatbuffers::FlatBufferBuilder builder{};

	/* Create ServerPacket */
	auto messageOffset = builder.CreateString(msg);
	auto ServerPacket  = FBProtocol::CreateCPkt_Chat(builder, messageOffset);
	builder.Finish(ServerPacket);

	/* Create SendBuffer */
	const uint8_t* bufferPointer      = builder.GetBufferPointer();
	const uint16_t SerializeddataSize = static_cast<uint16_t>(builder.GetSize());;
	SPtr_SendPktBuf sendBuffer        = ServerFBsPktFactory::MakeFBsSendPktBuf(bufferPointer, SerializeddataSize, ServerPacket);

	return sendBuffer;
}


/// +----------------------------
///			 CPkt_EnterGame
/// ----------------------------+
SPtr_SendPktBuf PacketFactory::CreateSendBuffer_CPkt_CEnterGame(uint64_t playerIdx)
{
	flatbuffers::FlatBufferBuilder builder;

	auto enterGamePkt = FBProtocol::CreateCPkt_EnterGame(builder, playerIdx);
	builder.Finish(enterGamePkt);

	const uint8_t* bufferPointer      = builder.GetBufferPointer();
	const uint16_t SerializeddataSize = static_cast<uint16_t>(builder.GetSize());;
	auto sendbuffer                   = ServerFBsPktFactory::MakeFBsSendPktBuf(bufferPointer, SerializeddataSize, enterGamePkt);

	return sendbuffer;
}



/// +----------------------------
///			 CPkt_Trnasform
/// ----------------------------+
SPtr_SendPktBuf PacketFactory::CreateSendBuffer_CPkt_Transform(Vec3 Pos, Vec3 Rot, Vec3 Scale, Vec3 SpineLookDir, long long timestamp)
{
	flatbuffers::FlatBufferBuilder builder;

	/* Create ServerPacket */
	auto position      = FBProtocol::CreateVector3(builder, Pos.x, Pos.y, Pos.z);
	auto rotation      = FBProtocol::CreateVector3(builder, Rot.x, Rot.y, Rot.z);
	auto scale         = FBProtocol::CreateVector3(builder, Scale.x, Scale.y, Scale.z);
	auto Spine_LookDir = FBProtocol::CreateVector3(builder, SpineLookDir.x, SpineLookDir.y, SpineLookDir.z);

	auto transform = FBProtocol::CreateTransform(builder, position, rotation, scale);

	auto ServerPacket = FBProtocol::CreateCPkt_Transform(builder, timestamp, transform, Spine_LookDir);
	builder.Finish(ServerPacket);

	/* Create SendBuffer */
	const uint8_t* bufferPointer      = builder.GetBufferPointer();
	const uint16_t SerializeddataSize = static_cast<uint16_t>(builder.GetSize());;
	SPtr_SendPktBuf sendBuffer        = ServerFBsPktFactory::MakeFBsSendPktBuf(bufferPointer, SerializeddataSize, ServerPacket);

	return sendBuffer;
}

/// +----------------------------
///			 CPkt_LogIn
/// ----------------------------+
SPtr_SendPktBuf PacketFactory::CreateSendBuffer_CPkt_LogIn(bool& success)
{
	flatbuffers::FlatBufferBuilder builder;

	auto pkt = FBProtocol::CreateCPkt_LogIn(builder);
	builder.Finish(pkt);

	const uint8_t* bufferPointer      = builder.GetBufferPointer();
	const uint16_t SerializeddataSize = static_cast<uint16_t>(builder.GetSize());;
	SPtr_SendPktBuf sendBuffer        = ServerFBsPktFactory::MakeFBsSendPktBuf(bufferPointer, SerializeddataSize, pkt);

	return sendBuffer;
}

SPtr_SendPktBuf PacketFactory::CreateSendBuffer_CPkt_KeyInput(GameKeyInfo::KEY key, GameKeyInfo::KEY_STATE KeyState, GameKeyInfo::MoveKey moveKey, Vec2 mouseDelta)
{
	flatbuffers::FlatBufferBuilder builder; 

	auto fbMouseDelta = FBProtocol::CreateVector2(builder, mouseDelta.x, mouseDelta.y);
	auto pkt = FBProtocol::CreateCPkt_KeyInput(builder, static_cast<UINT8>(KeyState), static_cast<UINT8>(key), static_cast<UINT8>(moveKey), fbMouseDelta);
	builder.Finish(pkt);

	const uint8_t* bufferPointer = builder.GetBufferPointer();
	const uint16_t SerializeddataSize = static_cast<uint16_t>(builder.GetSize());;
	SPtr_SendPktBuf sendBuffer = ServerFBsPktFactory::MakeFBsSendPktBuf(bufferPointer, SerializeddataSize, pkt);

	return sendBuffer;
}
