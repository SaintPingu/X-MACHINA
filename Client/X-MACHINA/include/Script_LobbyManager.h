#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion

class GameObject;


#pragma region Class
enum class TrooperSkin {
	Army, Dark, Desert, Forest, White, Winter
};

struct LobbyPlayerInfo {
	UINT32 ID{};
	std::string Name{};
	TrooperSkin Skin{};
};


class LobbyPlayer {
private:
	LobbyPlayerInfo mInfo{};
	GameObject* mObject{};

public:
	LobbyPlayer(const LobbyPlayerInfo& info, unsigned char idx);

	UINT32 GetID() const { return mInfo.ID; }
	const std::string GetName() const { return mInfo.Name; }
	GameObject* GetObj() const { return mObject; }

	void SetObject(GameObject* object) { mObject = object; }
	void SetSkin(TrooperSkin skin);
};


class Script_LobbyManager : public Component {
	COMPONENT(Script_LobbyManager, Component)

private:
	std::unordered_map<UINT32, sptr<LobbyPlayer>> mLobbyPlayers{};
	int mCurPlayerSize{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

public:
	void AddPlayer(const LobbyPlayerInfo& info);
	void RemovePlayer(UINT32 id);
	void ChangeSkin(UINT32 id, TrooperSkin skin);

	const std::unordered_map<UINT32, sptr<LobbyPlayer>>& GetPlayers() const { return mLobbyPlayers; }

public:
	static void ChangeToBattleScene();
};

#pragma endregion