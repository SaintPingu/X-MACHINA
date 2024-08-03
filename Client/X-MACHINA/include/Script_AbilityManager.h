#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class Script_RenderedAbility;
#pragma endregion



class Script_AbilityManager : public Component {
	COMPONENT(Script_AbilityManager, Component)

private:
	enum { MaxAbilityLayer = 10 };
	std::array<std::unordered_set<Script_RenderedAbility*>, MaxAbilityLayer> mRenderedAbilities;

public:
	virtual void Awake() override;

public:
	void AddRenderedAbilities(int layer, Script_RenderedAbility* ability) { mRenderedAbilities[layer].insert(ability); }
	void RemoveRenderedAbilities(int layer, Script_RenderedAbility* ability) { mRenderedAbilities[layer].erase(ability); }

	void Render();
};
