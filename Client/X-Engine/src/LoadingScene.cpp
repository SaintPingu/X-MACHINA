#include "EnginePch.h"
#include "LoadingScene.h"
#include "DXGIMgr.h"

#include "Component/UI.h"


void LoadingScene::Build()
{
	mBackground = Canvas::I->CreateUI<UI>(0, "LoadingBackground", Vec2::Zero, Vec2(DXGIMgr::I->GetWindowWidth(), DXGIMgr::I->GetWindowHeight()));
}

void LoadingScene::Release()
{
	Canvas::I->RemoveUI(0, mBackground);
}