#include "stdafx.h"
#include "Script_Ability_AerialController.h"

#include "Script_AerialCamera.h"
#include "Script_AimController.h"

#include "Component/Camera.h"
#include "Component/UI.h"

#include "BattleScene.h"
#include "ResourceMgr.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "Object.h"

void Script_Ability_AerialController::Awake()
{
	base::Awake();

	Init("AerialController", 10.f, 30.f);

	mLayer = 0;
	mAbilityCB.Duration = 29.8f;

	mRenderedMesh = RESOURCE<ModelObjectMesh>("Rect");
	mShader = RESOURCE<Shader>("AerialController");
	mAbilityCB.UIIndex = RESOURCE<Texture>("AerialControllerUI")->GetSrvIdx();
	mAbilityCB.NoiseIndex = RESOURCE<Texture>("Noise")->GetSrvIdx();
	mAbilityCB.UI2Index = RESOURCE<Texture>("FullNoise")->GetSrvIdx();

	mAimUI = Canvas::I->CreateUI<UI>(0, "AerialAim");
	mAimUI->SetActive(false);
}

void Script_Ability_AerialController::Start()
{
	base::Start();

	mAerialCamera = MainCamera::I->AddComponent<Script_AerialCamera>(true, false).get();
	mAimController = mObject->GetComponent<Script_AimController>().get();
}

bool Script_Ability_AerialController::ProcessInput()
{
	return true;
}

bool Script_Ability_AerialController::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	return true;
}

bool Script_Ability_AerialController::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	return true;
}

void Script_Ability_AerialController::On()
{
	base::On();
	BattleScene::I->SetFilterOptions(FilterOption::Custom);

	mAerialCamera->SetActive(true);
	ChangeAimToActive();
}

void Script_Ability_AerialController::Off()
{
	base::Off();
	BattleScene::I->SetFilterOptions(FilterOption::Custom);

	mAerialCamera->SetActive(false);
	ChangeAimToOrigin();
}

void Script_Ability_AerialController::ChangeAimToOrigin()
{
	mAimController->ChangeAimUI(mPrevAimUI);
	mAimUI->SetActive(false);
	mPrevAimUI->SetActive(true);
}

void Script_Ability_AerialController::ChangeAimToActive()
{
	mPrevAimUI = mAimController->GetUI();
	mAimController->ChangeAimUI(mAimUI);
	mAimUI->SetActive(true);
	mPrevAimUI->SetActive(false);
}

