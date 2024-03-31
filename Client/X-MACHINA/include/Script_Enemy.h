#pragma once


#pragma region Include
#include "Component/Component.h"
#include "Script_LiveObject.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
#pragma endregion


#pragma region Class
class Script_Enemy : public Script_LiveObject {
	COMPONENT(Script_Enemy, Script_LiveObject)

private:
	sptr<GridObject> mGun;

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

	virtual void OnDestroy() override;
};

#pragma endregion