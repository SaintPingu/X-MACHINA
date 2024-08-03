#include "EnginePch.h"
#include "LoadingScene.h"
#include "DXGIMgr.h"

#include "Component/UI.h"


void LoadingScene::Build()
{
	//base::Build();

	mBackground = Canvas::I->CreateUI<UI>(0, "LoadingBackground", Vec2::Zero, Vec2(DXGIMgr::I->GetWindowWidth(), DXGIMgr::I->GetWindowHeight()));
}

void LoadingScene::Release()
{
	//base::Release();

	Canvas::I->RemoveUI(0, mBackground);
}

void LoadingScene::RenderDeferred()
{
	base::RenderDeferred();

	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
