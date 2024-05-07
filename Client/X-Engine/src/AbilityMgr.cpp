#include "EnginePch.h"
#include "AbilityMgr.h"

#include "DXGIMgr.h"
#include "Mesh.h"
#include "Shader.h"
#include "Object.h"

void RenderedAbility::Update()
{
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

	if (mShader)
		mShader->Set();

	if (mRenderedObject)
		mRenderedObject->Render();

	if (mRenderedMesh)
		mRenderedMesh->Render();
}

void AbilityMgr::Render()
{
	for (const auto& layers : mRenderedAbilities) {
		for (const auto& ability : layers) {
			ability->Render();
		}
	}
}
