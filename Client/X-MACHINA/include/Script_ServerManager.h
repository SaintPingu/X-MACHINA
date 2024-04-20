#pragma once

#pragma region Include
#include "Component/Component.h"
#include "NetworkEvents.h"
#pragma endregion


class Script_ServerManager : public Component {
	COMPONENT(Script_ServerManager, Component)

private:
	USE_LOCK;
	struct PlayerUpdateInfo {
		sptr<GridObject> player;
		Vec3			 NewPos;
	};


	Concurrency::concurrent_unordered_map<UINT32, sptr<GridObject>> mPlayers{}; /* sessionID, otherPlayer */
	Concurrency::concurrent_queue<sptr<NetworkEvent::Scene::EventData>> mEventsProcessQueue{};

	//Concurrency::concurrent_queue<sptr<GridObject>> mEventsQueue_AddOtherPlayer;
	//Concurrency::concurrent_queue<PlayerUpdateInfo> mEventsQueue_MoveOtherPlayer;

public:
	virtual void Update() override;
	virtual void OnDestroy() override;

public:
	void ProcessEvents();
	void AddEvent(rsptr<NetworkEvent::Scene::EventData> data);
};