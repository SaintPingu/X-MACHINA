#include "EnginePch.h"
#include "LobbyScene.h"

#include "X-Engine.h"
#include "BattleScene.h"
#include "Object.h"
#include "InputMgr.h"
#include "TextMgr.h"
#include "DXGIMgr.h"

#include "Component/UI.h"

void LobbyScene::Init()
{
	// title
	Canvas::I->CreateUI(0, "Title", Vec2::Zero, Engine::I->GetWindowWidth(), Engine::I->GetWindowHeight());

	//sptr<GameObject> cursor = std::make_shared<GameObject>();
	//const auto& cursorUI = Canvas::I->CreateUI(3, "Aim", Vec2(0, 0), 30, 30);
	//cursor->AddComponent<Script_AimController>()->SetUI(cursorUI);
}

void LobbyScene::Update()
{
	Canvas::I->Update();

	if (KEY_TAP('S')) {
		Engine::I->LoadScene(SceneType::Battle);
		Canvas::I->RemoveUI(0, "Title");
	}
}

void LobbyScene::RenderBegin()
{
}

void LobbyScene::RenderShadow()
{
}

void LobbyScene::RenderDeferred()
{
	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void LobbyScene::RenderLights()
{
}

void LobbyScene::RenderCustomDepth()
{
}

void LobbyScene::RenderForward()
{
}


void LobbyScene::RenderUI()
{
	Canvas::I->Render();
}

void LobbyScene::RenderText(RComPtr<ID2D1DeviceContext2> device)
{
	TextMgr::I->Render(device);
}

void LobbyScene::ApplyDynamicContext()
{
}