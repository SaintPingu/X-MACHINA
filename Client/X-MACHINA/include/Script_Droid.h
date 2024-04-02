#pragma once


#pragma region Include
#include "Component/Component.h"
#include "Script_Enemy.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
class AnimatorController;
#pragma endregion


#pragma region Class
class Script_Droid : public Script_Enemy {
	COMPONENT(Script_Droid, Script_Enemy)

private:
	sptr<GridObject> mGun{};
	sptr<GridObject> mTarget{};

public:
	virtual void Awake() override;
	virtual void Update() override;

	virtual void OnDestroy() override;
};

#pragma endregion