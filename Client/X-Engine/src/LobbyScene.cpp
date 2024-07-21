#include "EnginePch.h"
#include "LobbyScene.h"

#include "X-Engine.h"
#include "Scene.h"
#include "InputMgr.h"

#include "Component/UI.h"

void LobbyScene::Init()
{
	// title
	Canvas::I->CreateUI(0, "Title", Vec2::Zero, Engine::I->GetWindowWidth(), Engine::I->GetWindowHeight());
}

void LobbyScene::Update()
{
	Canvas::I->Update();

	if (KEY_TAP('S')) {
		Engine::I->LoadScene(SceneType::Battle);
		Canvas::I->RemoveUI(0, "Title");
	}
}

void LobbyScene::Render()
{
	Canvas::I->Render();
}