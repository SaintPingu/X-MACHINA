#include "EnginePch.h"
#include "AbilityMgr.h"

#include "DXGIMgr.h"
#include "Mesh.h"
#include "Shader.h"
#include "Object.h"
#include "BattleScene.h"

void RenderedAbility::UpdateAbilityCB(float activeTime)
{
	mAbilityCB.ActiveTime = mActiveTime;
	mAbilityCB.AccTime = mActiveTime - activeTime;

	BattleScene::I->UpdateAbilityCB(mAbilityCBIdx, mAbilityCB);
}

void RenderedAbility::Update(float activeTime)
{
	UpdateAbilityCB(activeTime);
}

void RenderedAbility::Activate()
{
	AbilityMgr::I->AddRenderedAbilities(mLayer, shared_from_this());
}

void RenderedAbility::DeActivate()
{
	AbilityMgr::I->RemoveRenderedAbilities(mLayer, shared_from_this());
}

void RenderedAbility::Render()
{
	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (mShader) {
		mShader->Set();
	}

	if (mRenderedObject) {
		mRenderedObject->Render();
	}

	if (mRenderedMesh) {
		mRenderedMesh->Render();
	}
}

void AbilityMgr::Render()
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
