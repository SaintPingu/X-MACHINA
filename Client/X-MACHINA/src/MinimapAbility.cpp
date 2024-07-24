#include "stdafx.h"
#include "MinimapAbility.h"

#include "BattleScene.h"
#include "GameFramework.h"

#include "Mesh.h"
#include "Shader.h"
#include "Object.h"
#include "Texture.h"
#include "ResourceMgr.h"

#include "Component/UI.h"

MinimapAbility::MinimapAbility()
	:
	RenderedAbility("Minimap")
{
	mUI = Canvas::I->CreateUI<UI>(0, "Minimap", Vec2(0, 0), 1720 * 0.8f, 1100 * 0.8f);
	mUI->SetActive(false);

	mAbilityCB.MinimapData.MinimapSize = Vec2{ 1720.f, 1100.f };
	mAbilityCB.MinimapData.MinimapWorldSize = Vec2{ 224.f, 143.f };
	mAbilityCB.MinimapData.MinimapStartPos = Vec2{ -17.f, 195.f };
}

void MinimapAbility::Update(float activeTime)
{
	const Vec3 playerPos = GameFramework::I->GetPlayer()->GetPosition().xz();
	mAbilityCB.MinimapData.PlayerPos = Vec2{ playerPos.x, playerPos.z };

	base::Update(activeTime);
}

void MinimapAbility::Activate()
{
	mUI->SetActive(true);

	base::Activate();
	BattleScene::I->SetFilterOptions(FilterOption::Blur);
}

void MinimapAbility::DeActivate()
{
	mUI->SetActive(false);

	base::DeActivate();
	BattleScene::I->SetFilterOptions(FilterOption::Blur);
}

void MinimapAbility::Render()
{
}
