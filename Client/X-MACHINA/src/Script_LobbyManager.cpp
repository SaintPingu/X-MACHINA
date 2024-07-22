#include "stdafx.h"
#include "Script_LobbyManager.h"

#include "Script_AimController.h"

#include "X-Engine.h"
#include "InputMgr.h"
#include "Scene.h"
#include "LobbyScene.h"

#include "Component/Camera.h"
#include "Component/UI.h"



void Script_LobbyManager::Awake()
{
	base::Awake();

	MainCamera::I->SetPosition(Vec3(1, 2, 2));
	MainCamera::I->LookAt({ 0, 1, 0 }, Vector3::Up);
	MAIN_CAMERA->SetProjMtx(0.01f, 200.f, 60.f);
}

void Script_LobbyManager::Start()
{
	base::Start();

	const auto& ui = Canvas::I->CreateUI(0, "Title", { -1100, 0 }, 800, 600);
	ui->AddClickCallback(std::bind(&Script_LobbyManager::Test, this));

	mCursor = Canvas::I->CreateUI(0, "Cursor", Vec2::Zero, 60, 60);
	LobbyScene::I->GetManager()->AddComponent<Script_AimController>()->SetUI(mCursor);

	LobbyScene::I->Instantiate("EliteTrooper");
}

void Script_LobbyManager::Update()
{
	base::Update();

	if (KEY_TAP('Q')) {
		Engine::I->LoadScene(SceneType::Battle);
	}
	if (KEY_TAP(VK_LBUTTON)) {
		Canvas::I->CheckClick(Vec2(mCursor->GetPosition()));
;	}
}

void Script_LobbyManager::Reset()
{
	base::Reset();

	mObject->RemoveComponent<Script_AimController>();
}

void Script_LobbyManager::Test()
{
	std::cout << "click\n";
}