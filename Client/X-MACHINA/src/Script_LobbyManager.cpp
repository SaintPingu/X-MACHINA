#include "stdafx.h"
#include "Script_LobbyManager.h"

#include "Script_AimController.h"

#include "X-Engine.h"
#include "InputMgr.h"
#include "Scene.h"
#include "LobbyScene.h"
#include "GameFramework.h"

#include "Component/Camera.h"
#include "Component/UI.h"



void Script_LobbyManager::Awake()
{
	base::Awake();

	MainCamera::I->SetPosition(Vec3(10, 5, 3));
	MainCamera::I->LookAt({ 0, 1, 0 }, Vector3::Up);
	MAIN_CAMERA->SetProjMtx(0.01f, 200.f, 60.f);
}

void Script_LobbyManager::Start()
{
	base::Start();

	const auto& ui = Canvas::I->CreateUI(0, "Title", { -1100, 0 }, 200, 100);
	ui->AddClickCallback(std::bind(&Script_LobbyManager::ChangeToBattleScene, this));

	mCursorNormal = Canvas::I->CreateUI(4, "Cursor_Normal", Vec2::Zero, 60, 60);
	mCursorClick = Canvas::I->CreateUI(4, "Cursor_Click", Vec2::Zero, 60, 60);
	mCursorClick->SetActive(false);

	mAimController = LobbyScene::I->GetManager()->AddComponent<Script_AimController>();
	mAimController->SetUI(mCursorNormal);

	LobbyScene::I->Instantiate("EliteTrooper");
}

void Script_LobbyManager::Update()
{
	base::Update();

	if (KEY_TAP('Q')) {
		ChangeToBattleScene();
	}

	if (KEY_TAP(VK_LBUTTON)) {
		Vec2 pos = mAimController->GetAimNDCPos();
		Canvas::I->CheckClick(pos);

		mCursorNormal->SetActive(false);
		mCursorClick->SetActive(true);
		mAimController->SetUI(mCursorClick);
;	}
	else if (KEY_AWAY(VK_LBUTTON)) {
		Vec2 pos = mAimController->GetAimNDCPos();

		mCursorNormal->SetActive(true);
		mCursorClick->SetActive(false);
		mAimController->SetUI(mCursorNormal);
	}
}

void Script_LobbyManager::Reset()
{
	base::Reset();

	mObject->RemoveComponent<Script_AimController>();
}

void Script_LobbyManager::ChangeToBattleScene()
{
	Engine::I->LoadScene(SceneType::Battle);
}