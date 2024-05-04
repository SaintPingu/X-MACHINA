#pragma once

//#define CONNECT_WITH_TEST_CLIENT
#ifdef CONNECT_WITH_TEST_CLIENT

namespace FBsProtocolID
{
	constexpr UINT16 CPkt_LogIn          = 1000;
	constexpr UINT16 SPkt_LogIn          = 1001;

	constexpr UINT16 CPkt_EnterGame      = 1002;
	constexpr UINT16 SPkt_EnterGame      = 1003;

	constexpr UINT16 CPkt_Chat           = 1004;
	constexpr UINT16 SPkt_Chat           = 1005;

	constexpr UINT16 CPkt_Transform      = 1006;
	constexpr UINT16 SPkt_Transform      = 1007;

	constexpr UINT16 CPkt_NetworkLatency = 1008;
	constexpr UINT16 SPkt_NetworkLatency = 1009;

	/* JUST FOR DEBUGGING NOT USE WITH TEST CLIENT PROTOCOL */
	constexpr UINT16 CPkt_NewPlayer = 0;
	constexpr UINT16 SPkt_NewPlayer = 0;
};
#else 
/* CONNECT WITH X-MACHINA */
namespace FBsProtocolID
{
	constexpr UINT16 CPkt_LogIn				= 1000;
	constexpr UINT16 SPkt_LogIn				= 1001;

	constexpr UINT16 CPkt_NewPlayer			= 1002;
	constexpr UINT16 SPkt_NewPlayer			= 1003;

	constexpr UINT16 CPkt_RemovePlayer		= 1004;
	constexpr UINT16 SPkt_RemovePlayer		= 1005;

	constexpr UINT16 CPkt_EnterGame			= 1006;
	constexpr UINT16 SPkt_EnterGame			= 1007;

	constexpr UINT16 CPkt_Chat				= 1008;
	constexpr UINT16 SPkt_Chat				= 1009;

	constexpr UINT16 CPkt_Transform			= 1010;
	constexpr UINT16 SPkt_Transform			= 1011;

	constexpr UINT16 CPkt_KeyInput			= 1012;
	constexpr UINT16 SPkt_KeyInput			= 1013;

	constexpr UINT16 CPkt_NetworkLatency	= 1014;
	constexpr UINT16 SPkt_NetworkLatency	= 1015;

	constexpr UINT16 CPkt_PlayerState		= 1016;
	constexpr UINT16 SPkt_PlayerState       = 1017;



};
#endif


struct PacketHeader
{
	UINT16 PacketSize  = 0;
	UINT16 ProtocolID  = 0;
};

