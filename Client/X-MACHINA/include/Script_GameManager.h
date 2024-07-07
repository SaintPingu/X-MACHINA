#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
class GameObject;
class Script_MainCamera;
#pragma endregion


#pragma region Class
class Script_GameManager : public Component {
	COMPONENT(Script_GameManager, Component)

private:
	sptr<Script_MainCamera> mMainCamera{};
	std::vector<sptr<GameObject>> mObjects{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

	rsptr<Script_MainCamera> GetCamera() const { return mMainCamera; }

private:
	void InitSceneObjectScripts();
	void InitCustomObjectScripts();

	void ProcessSceneObjectScript(sptr<GameObject> object);
};

#pragma endregion