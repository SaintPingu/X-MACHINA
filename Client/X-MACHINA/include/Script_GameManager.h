#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
#pragma endregion


#pragma region Class
class Script_GameManager : public Component {
	COMPONENT(Script_GameManager, Component)

private:
	std::vector<sptr<GameObject>> mObjects;

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

private:
	void InitObjectScripts();
};

#pragma endregion