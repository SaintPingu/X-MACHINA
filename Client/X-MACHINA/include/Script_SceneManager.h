#pragma once


#pragma region Include
#include "Singleton.h"
#pragma endregion

class Script_LoginManager;
class Script_LobbyManager;
class Script_BattleManager;


#pragma region Class
class Script_SceneManager : public Singleton<Script_SceneManager> {
	friend Singleton;

private:
	sptr<Script_LoginManager> mLoginManager;
	sptr<Script_LobbyManager> mLobbyManager;
	sptr<Script_BattleManager> mBattleManager;

public:
	rsptr<Script_LoginManager> LoginManager() const { return mLoginManager; }
	rsptr<Script_LobbyManager> LobbyManager() const { return mLobbyManager; }
	rsptr<Script_BattleManager> BattleManager() const { return mBattleManager; }

	void SetLoginManager(rsptr<Script_LoginManager> manager) { mLoginManager = manager; }
	void SetLobbyManager(rsptr<Script_LobbyManager> manager) { mLobbyManager = manager; }
	void SetBattleManager(rsptr<Script_BattleManager> manager) { mBattleManager = manager; }
};

#pragma endregion