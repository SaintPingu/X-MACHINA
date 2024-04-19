#include "stdafx.h"
#include "ServerFBsPktFactory.h"

#include "Script_GameManager.h"
#include "Script_Player.h"
#include "Script_ServerManager.h"


#include "Enum_generated.h"
#include "FBProtocol_generated.h"
#include "Struct_generated.h"
#include "Transform_generated.h"
#include "../PacketFactory.h"
#include "Scene.h"
#include "framework.h"
#include "Object.h"
#include "Scene.h"



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

	/* My Client Info ( in server ) */
	const FBProtocol::Player* MySessionInfo = pkt.myinfo();
	std::string				  Myname        = MySessionInfo->name()->c_str();
	UINT32					  MysessionID   = MySessionInfo->id();
	FBProtocol::OBJECTTYPE	  MyobjType     = MySessionInfo->player_type();

	std::cout << "����������������������������������������������������������������������\n";
	std::cout << Myname << " - " << MysessionID << " \n";
	std::cout << "����������������������������������������������������������������������\n";

	/* ���� �ڽ��� ������ ������ Room ������ ���� ��� Ŭ���̾�Ʈ ������ �޴´�. (�ٸ� ������ ���̵� ���� )*/
	int PlayersCnt = pkt.players()->size();
	for (UINT32 i = 0; i < PlayersCnt; ++i) {

		const FBProtocol::Player* otherPlayerInfo = pkt.players()->Get(i);
		UINT32					  sessionID   = otherPlayerInfo->id();
		if (sessionID == MysessionID) continue;
		std::string				  name        = otherPlayerInfo->name()->c_str();
		FBProtocol::OBJECTTYPE	  objType     = otherPlayerInfo->player_type();
		std::cout << " ���� ���� ���� ������Ʈ - " << sessionID << "\n";
		
		
		/* GameScene �� �ٸ� Player �������� �����.  */
		/* Create Other Player & Add To Game Scene */
		sptr<GridObject> otherPlayer = Scene::I->Instantiate("EliteTrooper");
		otherPlayer->SetName(name);
		otherPlayer->SetID(sessionID);

		sptr<SceneEvent::AddOtherPlayer> EventData = std::make_shared<SceneEvent::AddOtherPlayer>();
		EventData->type = SceneEvent::Enum::AddAnotherPlayer;
		EventData->player = otherPlayer;
		SERVER_MGR->GetComponent<Script_ServerManager>()->AddEvent(EventData);


	}

	// ���� UI ��ư ������ ���� ����
	uint64_t playerIdx = 0; /* �ӽ� */
	auto CPktBuf       = PacketFactory::CreateSendBuffer_CPkt_CEnterGame(playerIdx);
	session->Send(CPktBuf);
	std::cout << "LogIn End\n";

	return true;
}

bool ProcessFBsPkt_SPkt_NewPlayer(SPtr_PacketSession& session, const FBProtocol::SPkt_NewPlayer& pkt)
{
	/* ���� ���� player �� ������ �޴´�. */
		/* New Client Info ( in server ) */
	const FBProtocol::Player* NewSessionInfo = pkt.newplayer();
	std::string				  Newname        = NewSessionInfo->name()->c_str();
	UINT32					  NewsessionID   = NewSessionInfo->id();
	FBProtocol::OBJECTTYPE	  NewobjType     = NewSessionInfo->player_type();

	std::cout << "�âââââââââââââââââââââââââââââââââ�\n";
	std::cout << "New Session Enter : " << Newname << " - " << NewsessionID << "\n";
	std::cout << "�âââââââââââââââââââââââââââââââââ�\n";

	/* Create Other Player & Add To Game Scene */
	sptr<GridObject> otherPlayer = Scene::I->Instantiate("EliteTrooper");
	otherPlayer->SetName(Newname);
	otherPlayer->SetID(NewsessionID);

	sptr<SceneEvent::AddOtherPlayer> EventData = std::make_shared<SceneEvent::AddOtherPlayer>();
	EventData->type                            = SceneEvent::Enum::AddAnotherPlayer;
	EventData->player                          = otherPlayer;

	SERVER_MGR->GetComponent<Script_ServerManager>()->AddEvent(EventData);

	std::cout << "New Session Enter End\n";
	return true;
}

bool ProcessFBsPkt_SPkt_EnterGame(SPtr_PacketSession& session, const FBProtocol::SPkt_EnterGame& pkt)
{
	return true;
}

bool ProcessFBsPkt_SPkt_Chat(SPtr_PacketSession& session, const FBProtocol::SPkt_Chat& pkt)
{
	std::cout << "[CHAT] : " << pkt.message()->c_str() << std::endl;

	//std::cout << "--->[RECV]" << pkt.message()->c_str() << std::endl;
	return true;
}

bool ProcessFBsPkt_SPkt_Transform(SPtr_PacketSession& session, const FBProtocol::SPkt_Transform& pkt)
{
	// ��Ŷ���� ��ü ID�� ��ġ ���� ��������
	int objID = pkt.object_id();

	// ��Ŷ���� ��ġ ������ �ִ��� Ȯ���ϰ� ������

	Vec3 Pos = { pkt.trans()->position()->x() ,pkt.trans()->position()->y() ,pkt.trans()->position()->z() };
	Vec3 Rot = { pkt.trans()->rotation()->x() ,pkt.trans()->rotation()->y() ,pkt.trans()->rotation()->z() };

	sptr<SceneEvent::MoveOtherPlayer> EventData = std::make_shared<SceneEvent::MoveOtherPlayer>();
	EventData->type                             = SceneEvent::Enum::MoveOtherPlayer;
	EventData->sessionID                        = objID;
	EventData->Pos                              = Pos;
	//std::cout << "�� ID : " << objID << " " << " POS - (x: " << Pos.x << ", y: " << Pos.y << ", z: " << Pos.z << ")\n";
	SERVER_MGR->GetComponent<Script_ServerManager>()->AddEvent(EventData);


	//std::cout << "--------------------------------------------------------------------\n";
	//std::cout << "�� Server Packet ( SPkt_TRansform ) \n";
	//std::cout << "Object ID: " << objID << "\n" << 
	//			" POS - (x: " << Pos.x << ", y: " << Pos.y << ", z: " << Pos.z << ")\n" <<
	//			" ROT - (x: " << Rot.x << ", y: " << Rot.y << ", z: " << Rot.z << ")\n";
	//std::cout << "--------------------------------------------------------------------\n";


	return true;
}

bool ProcessFBsPkt_SPkt_KeyInput(SPtr_PacketSession& session, const FBProtocol::SPkt_KeyInput& pkt)
{
	return false;
}
