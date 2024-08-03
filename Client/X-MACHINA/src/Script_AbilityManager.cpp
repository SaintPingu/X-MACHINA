#include "stdafx.h"
#include "Script_AbilityManager.h"

#include "Script_Ability.h"
#include "BattleScene.h"


void Script_AbilityManager::Awake()
{
	base::Awake();

	BattleScene::I->SetRenderForwardCallback(std::bind(&Script_AbilityManager::Render, this));
}

void Script_AbilityManager::Render()
{
	for (const auto& layers : mRenderedAbilities) {
		for (const auto& ability : layers) {
			const int abilityCBIdx = ability->GetAbilityCBIdx();

			if (abilityCBIdx == -1) {
				continue;
			}

			BattleScene::I->SetAbilityCB(abilityCBIdx);
			ability->Render();
		}
	}
}
