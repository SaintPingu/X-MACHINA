#include "EnginePch.h"
#include "LoadingScene.h"
#include "DXGIMgr.h"

#include "Component/UI.h"


void LoadingScene::Build()
{
	base::Build();

	mName = "Loading";
	Canvas::I->CreateUI<UI>(0, "LoadingBackground", Vec2::Zero, Vec2(Canvas::I->GetWidth(), Canvas::I->GetHeight()));
}

void LoadingScene::RenderDeferred()
{
	base::RenderDeferred();

	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
