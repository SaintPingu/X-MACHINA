#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
class Script_MainCamera;
class Script_AbilityManager;
class Script_BattleUI;
#pragma endregion


#pragma region Class
class Script_BattleManager : public Component {
	COMPONENT(Script_BattleManager, Component)

private:
	GameObject* mPlayer{};
	sptr<Script_MainCamera> mMainCamera{};
	std::vector<sptr<GameObject>> mObjects{};
	sptr<Script_AbilityManager> mAbilityMgr{};
	Script_BattleUI* mUI{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void Reset() override;

	Script_BattleUI* GetUI() const { return mUI; }
	rsptr<Script_MainCamera> GetCamera() const { return mMainCamera; }
	rsptr<Script_AbilityManager> GetAbilityMgr() const { return mAbilityMgr; }

	void OnUI();
	void OffUI();

private:
	void InitComponents();
	void InitPlayers();
	void InitSceneObjectScripts();
	void InitCustomObjectScripts();

	void ProcessSceneObjectScript(sptr<Object> object);

	void WriteBounds(rsptr<Object> object);
};

#pragma endregion