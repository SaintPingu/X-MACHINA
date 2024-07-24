#include "stdafx.h"
#include "Script_LobbyManager.h"

#include "Script_AimController.h"

#include "X-Engine.h"
#include "InputMgr.h"
#include "TextMgr.h"
#include "Timer.h"
#include "Scene.h"
#include "Light.h"
#include "Object.h"
#include "LobbyScene.h"
#include "GameFramework.h"

#include "Component/Camera.h"
#include "Component/UI.h"



void Script_LobbyManager::Awake()
{
	base::Awake();

	//MainCamera::I->SetPosition(Vec3(5, 3, 2));
	MAIN_CAMERA->SetOffset(Vec3(9.88f, 1.86f, 6.93f));
	MainCamera::I->SetPosition(Vec3(9.88f, 1.86f, 6.93f));
	MainCamera::I->SetLocalRotation(Quaternion::ToQuaternion(Vec3(8.25f, -124.f, 0.f)));
	MainCamera::I->LookAt({ 0, 1, 0 }, Vector3::Up);
	MAIN_CAMERA->SetProjMtx(0.01f, 200.f, 60.f);
}

void Script_LobbyManager::Start()
{
	base::Start();

	const auto& start = Canvas::I->CreateUI(0, "Title", { -1100, 0 }, 200, 100);
	start->AddClickCallback(std::bind(&Script_LobbyManager::ChangeToBattleScene, this));

	//const auto& menu = Canvas::I->CreateUI(0, "Menu", { 0, 1080 - 150 }, 1920, 150);
	const auto& layout = Canvas::I->CreateUI(0, "Layout", { 0, 0 }, 1920, 1080);

	mCursorNormal = Canvas::I->CreateUI(4, "Cursor_Normal", Vec2::Zero, 60, 60);
	mCursorClick = Canvas::I->CreateUI(4, "Cursor_Click", Vec2::Zero, 60, 60);
	mCursorClick->SetActive(false);

	{
		TextOption textOption;
		textOption.FontSize = 72.f;
		textOption.FontColor = TextFontColor::Type::WhiteSmoke;
		textOption.FontWeight = TextFontWeight::EXTRA_BOLD;

		TextMgr::I->CreateText("PLAY", Vec2(0, -480), textOption);
	}

	mAimController = LobbyScene::I->GetManager()->AddComponent<Script_AimController>();
	mAimController->SetUI(mCursorNormal);

	const auto& trooper = LobbyScene::I->Instantiate("EliteTrooper");
	trooper->SetPosition(7.4f, 0, 5.27f);
	trooper->SetLocalRotation(Quaternion::ToQuaternion(Vec3(0.f, 53.41f, 0.f)));

	LobbyScene::I->GetLight()->SetSunlightDir(Vec3(-1, -2, -2));
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