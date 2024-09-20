#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion

class GameObject;


#pragma region Class
struct LobbyPlayerInfo {
	UINT32 ID;
	std::string Name;
};


class LobbyPlayer {
private:
	LobbyPlayerInfo mInfo{};
	GameObject* mObject{};

public:
	LobbyPlayer(const LobbyPlayerInfo& info);

	UINT32 GetID() const { return mInfo.ID; }
	const std::string GetName() const { return mInfo.Name; }
	GameObject* GetObj() const { return mObject; }

	void SetObject(GameObject* object) { mObject = object; }
};


class Script_LobbyManager : public Component {
	COMPONENT(Script_LobbyManager, Component)

private:
	std::unordered_map<UINT32, sptr<LobbyPlayer>> mLobbyPlayers{};
	
public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

public:
	void AddPlayer(const LobbyPlayerInfo& info);
	void RemovePlayer(UINT32 id);

	const std::unordered_map<UINT32, sptr<LobbyPlayer>>& GetPlayers() const { return mLobbyPlayers; }

public:
	static void ChangeToBattleScene();
};

#pragma endregion