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
    /// +-----------------------------------------------------------------------
    ///  PLAYER PACKET HEADER PROTOCOL TYPE
    /// -----------------------------------------------------------------------+
    constexpr UINT16 CPkt_NewPlayer = 1;
    constexpr UINT16 SPkt_NewPlayer = 2;

    constexpr UINT16 CPkt_RemovePlayer = 3;
    constexpr UINT16 SPkt_RemovePlayer = 4;

    constexpr UINT16 CPkt_PlayerOnSkill = 5;
    constexpr UINT16 SPkt_PlayerOnSkill = 6;

    constexpr UINT16 CPkt_Player_Transform = 7;
    constexpr UINT16 SPkt_Player_Transform = 8;

    constexpr UINT16 CPkt_Player_Animation = 9;
    constexpr UINT16 SPkt_Player_Animation = 10;

    constexpr UINT16 CPkt_Player_Weapon = 11;
    constexpr UINT16 SPkt_Player_Weapon = 12;

    /// +-----------------------------------------------------------------------
    ///  MONSTER PACKET HEADER PROTOCOL TYPE
    /// -----------------------------------------------------------------------+
    constexpr UINT16 CPkt_NewMonster = 13;
    constexpr UINT16 SPkt_NewMonster = 14;

    constexpr UINT16 CPkt_DeadMonster = 15;
    constexpr UINT16 SPkt_DeadMonster = 16;

    constexpr UINT16 CPkt_RemoveMonster = 17;
    constexpr UINT16 SPkt_RemoveMonster = 18;

    constexpr UINT16 CPkt_Monster_Transform = 19;
    constexpr UINT16 SPkt_Monster_Transform = 20;

    constexpr UINT16 CPkt_Monster_HP = 21;
    constexpr UINT16 SPkt_Monster_HP = 22;

    constexpr UINT16 CPkt_Monster_State = 23;
    constexpr UINT16 SPkt_Monster_State = 24;

    /// +-----------------------------------------------------------------------
///  PHERO PACKET HEADER PROTOCOL TYPE
/// -----------------------------------------------------------------------+
    constexpr UINT16 CPkt_GetPhero = 25;
    constexpr UINT16 SPkt_GetPhero = 26;

    /// +-----------------------------------------------------------------------
    ///  BULLET PACKET HEADER PROTOCOL TYPE
    /// -----------------------------------------------------------------------+
    constexpr UINT16 CPkt_Bullet_OnShoot = 27;
    constexpr UINT16 SPkt_Bullet_OnShoot = 28;

    constexpr UINT16 CPkt_Bullet_OnCollision = 29;
    constexpr UINT16 SPkt_Bullet_OnCollision = 30;
};
#else 
/* CONNECT WITH X-MACHINA */
namespace FBsProtocolID
{
    constexpr UINT16 CPkt_LogIn                 = 1000;
    constexpr UINT16 SPkt_LogIn                 = 1001;

    constexpr UINT16 CPkt_Chat                  = 1002;
    constexpr UINT16 SPkt_Chat                  = 1003;

    constexpr UINT16 CPkt_NetworkLatency        = 1004;
    constexpr UINT16 SPkt_NetworkLatency        = 1005;

    constexpr UINT16 CPkt_EnterGame             = 1006;
    constexpr UINT16 SPkt_EnterGame             = 1007;

    /// +-----------------------------------------------------------------------
    ///  PLAYER PACKET HEADER PROTOCOL TYPE
    /// -----------------------------------------------------------------------+
    constexpr UINT16 CPkt_NewPlayer             = 1008;
    constexpr UINT16 SPkt_NewPlayer             = 1009;

    constexpr UINT16 CPkt_RemovePlayer          = 1010;
    constexpr UINT16 SPkt_RemovePlayer          = 1011;

    constexpr UINT16 CPkt_PlayerOnSkill         = 1012;
    constexpr UINT16 SPkt_PlayerOnSkill         = 1013;

    constexpr UINT16 CPkt_Player_Transform      = 1014;
    constexpr UINT16 SPkt_Player_Transform      = 1015;

    constexpr UINT16 CPkt_Player_Animation      = 1016;
    constexpr UINT16 SPkt_Player_Animation      = 1017;

    constexpr UINT16 CPkt_Player_Weapon         = 1018;
    constexpr UINT16 SPkt_Player_Weapon         = 1019;

    constexpr UINT16 CPkt_Player_AimRotation    = 1020;
    constexpr UINT16 SPkt_Player_AimRotation    = 1021;

    constexpr UINT16 CPkt_Player_State          = 1022;
    constexpr UINT16 SPkt_Player_State          = 1023;


    /// +-----------------------------------------------------------------------
    ///  MONSTER PACKET HEADER PROTOCOL TYPE
    /// -----------------------------------------------------------------------+
    constexpr UINT16 CPkt_NewMonster            = 1024;
    constexpr UINT16 SPkt_NewMonster            = 1025;

    constexpr UINT16 CPkt_DeadMonster           = 1026;
    constexpr UINT16 SPkt_DeadMonster           = 1027;

    constexpr UINT16 CPkt_RemoveMonster         = 1028;
    constexpr UINT16 SPkt_RemoveMonster         = 1029;

    constexpr UINT16 CPkt_Monster_Transform     = 1030;
    constexpr UINT16 SPkt_Monster_Transform     = 1031;

    constexpr UINT16 CPkt_Monster_HP            = 1032;
    constexpr UINT16 SPkt_Monster_HP            = 1033;

    constexpr UINT16 CPkt_Monster_State         = 1034;
    constexpr UINT16 SPkt_Monster_State         = 1035;    
    
    constexpr UINT16 CPkt_Monster_Target        = 1036;
    constexpr UINT16 SPkt_Monster_Target        = 1037;

    /// +-----------------------------------------------------------------------
    ///  PHERO PACKET HEADER PROTOCOL TYPE
    /// -----------------------------------------------------------------------+
    constexpr UINT16 CPkt_GetPhero              = 1038;
    constexpr UINT16 SPkt_GetPhero              = 1039;

    /// +-----------------------------------------------------------------------
    ///  BULLET PACKET HEADER PROTOCOL TYPE
    /// -----------------------------------------------------------------------+
    constexpr UINT16 CPkt_Bullet_OnShoot        = 1040;
    constexpr UINT16 SPkt_Bullet_OnShoot        = 1041;

    constexpr UINT16 CPkt_Bullet_OnHitEnemy     = 1041;
    constexpr UINT16 SPkt_Bullet_OnHitEnemy     = 1042;

    constexpr UINT16 CPkt_Bullet_OnCollision    = 1043;
    constexpr UINT16 SPkt_Bullet_OnCollision    = 1044;

    /// +-----------------------------------------------------------------------
    ///  ITEM PACKET HEADER PROTOCOL TYPE
    /// -----------------------------------------------------------------------+
    constexpr UINT16 CPkt_Item_Interact         = 1045;
    constexpr UINT16 SPkt_Item_Interact         = 1046;

    constexpr UINT16 CPkt_Item_ThrowAway        = 1047;
    constexpr UINT16 SPkt_Item_ThrowAway        = 1048;

}


#endif


struct PacketHeader
{
	UINT16 PacketSize  = 0;
	UINT16 ProtocolID  = 0;
};

