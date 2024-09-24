#include "stdafx.h"
#include "Script_PlayerController.h"

#include "Script_Player.h"
#include "Script_FootStepSound.h"
#include "Script_AimController.h"

#include "Script_Ability_Shield.h"
#include "Script_Ability_IRDetector.h"
#include "Script_Ability_MindControl.h"
#include "Script_Ability_Cloaking.h"
#include "Script_Ability_NightVision.h"
#include "Script_Ability_AerialController.h"

#include "ChatBoxUI.h"
#include "SliderBarUI.h"
#include "InputMgr.h"

#include "Component/UI.h"

#include "Timer.h"

void Script_PlayerController::Awake()
{
	base::Awake();

	mObject->AddComponent<Script_FootStepSound>();

	const auto& aimUI = Canvas::I->CreateUI<UI>(3, "Aim", Vec2::Zero, Vec2(45, 45));
	mAimController = mObject->AddComponent<Script_AimController>();
	mAimController->SetIconUI(aimUI);

	mAimHitUI = Canvas::I->CreateUI<UI>(4, "Aim_Hit", Vec2::Zero, Vec2(45, 45));
	mAimHitUI->SetActive(false);
	mAimHitUI->SetOpacity(0.f);


	mAbilityShield           = mObject->AddComponent<Script_Ability_Shield>(true, false);
	mAbilityIRDetector       = mObject->AddComponent<Script_Ability_IRDetector>(true, false);
	mRemoteAbilityMindControl      = mObject->AddComponent<Script_Ability_MindControl>(true, false);
	mAbilityCloaking         = mObject->AddComponent<Script_Ability_Cloaking>(true, false);
	mAbilityNightVision      = mObject->AddComponent<Script_Ability_NightVision>(true, false);
	mAbilityAerialController = mObject->AddComponent<Script_Ability_AerialController>(true, false);

	mHurtUI = Canvas::I->CreateUI<UI>(0, "HurtUI");
	mHurtUIdelta = 0.f;
	mHurtUI->SetOpacity(0.f);
}

void Script_PlayerController::Start()
{
	base::Start();

	mChatBoxUI = std::make_shared<ChatBoxUI>(Vec2{ -775.f, -380.f }, Vec2{ 300.f, 200.f }, mObject->GetName());

	mScript = mObject->GetComponent<Script_PheroPlayer>();

	mHpBarUI = std::make_shared<SliderBarUI>("BackgroundHpBar", "EaseBar", "Image", Vec2{ 0.f, -480.f }, Vec2{ 500.f, 15.f }, mScript->GetMaxHp());
	mPheroBarUI = std::make_shared<SliderBarUI>("BackgroundPheroBar", "EaseBar", "Image", Vec2{ 0.f, -500.f }, Vec2{ 500.f, 15.f }, mScript->GetMaxPheroAmount());

	mHpBarUI->SetDissolve(Vec3(1, 0.23f, 0.19f));
	mPheroBarUI->SetDissolve(Vec3(0.54f, 0.19f, 0.89f));
}

void Script_PlayerController::Update()
{
	base::Update();

	ProcessInput();

	UpdateUI();
}


bool Script_PlayerController::IsActiveChatBox() const
{
	return mChatBoxUI->IsActive();
}

bool Script_PlayerController::ProcessInput()
{
	if (!mScript || mChatBoxUI->IsActive()) {
		return false;
	}

	if (!IsInAerialControl() || !IsInMindControl()) {
		mScript->ProcessInput();
	}

	return true;
}

bool Script_PlayerController::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	if (!mScript) {
		return false;
	}

	if (IsInAerialControl()) {
		mAbilityAerialController->ProcessMouseMsg(messageID, wParam, lParam);
	}
	else if (IsInMindControl()) {
		mRemoteAbilityMindControl->ProcessMouseMsg(messageID, wParam, lParam);
	}
	else {
		mScript->ProcessMouseMsg(messageID, wParam, lParam);
	}


	return true;
}

#include "Component/ParticleSystem.h"

bool Script_PlayerController::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	if (!mScript) {
		return false;
	}

	if (mChatBoxUI && mChatBoxUI->IsActive()) {
		mChatBoxUI->ProcessKeyboardMsg(messageID, wParam, lParam);
		return true;
	}

	if (!IsInAerialControl()) {
		mScript->ProcessKeyboardMsg(messageID, wParam, lParam);
	}

	switch (messageID) {
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_RETURN:
			mChatBoxUI->ToggleChatBox();
			return false;

			// weapons //
		case 'G':
			mScript->DropCrntWeapon();
			break;

			// phero skills //
		case 'U':
			mRemoteAbilityMindControl->Toggle();
			break;
		case 'T':
			mAbilityShield->Toggle();
			break;
		case 'I':
			mAbilityCloaking->Toggle();
			break;
		case 'Y':
			mAbilityIRDetector->Toggle();
			break;
		case 'N':
			mAbilityNightVision->Toggle();
			break;
		case 'M':
			mAbilityAerialController->Toggle();
			if (!mAbilityAerialController->IsActiveState()) {
				mScript->SendCrntWeapon();
			}
			break;

		//	// cheats //
		//case '6':
		//	mScript->AquireNewWeapon(WeaponName::SkyLine);
		//	break;
		//case '7':
		//	mScript->AquireNewWeapon(WeaponName::DBMS);
		//	break;
		//case '8':
		//	mScript->AquireNewWeapon(WeaponName::Burnout);
		//	break;
		//case '9':
		//	mScript->AquireNewWeapon(WeaponName::PipeLine);
		//	break;

		case'L':
			mScript->AddPheroAmount(1000);
			break;
		case'J':
			mScript->Hit(100);
			Hit();
			break;
		case'H':
			mScript->FillHP(1000);
			break;
		}
	}
	break;
	}

	return true;
}

void Script_PlayerController::Chat(const std::string& text, const std::string& playerName)
{
	mChatBoxUI->AddChat(text, playerName);
}

void Script_PlayerController::Hit()
{
	mHurtUIdelta = 1.f;
}

void Script_PlayerController::ActiveHitAim()
{
	mAimHitUI->SetActive(true);
	mHitUIDelta = 1.f;
	mAimHitUI->SetOpacity(mHitUIDelta);
}

bool Script_PlayerController::IsInAerialControl()
{
	return mAbilityAerialController && mAbilityAerialController->IsActiveState();
}

bool Script_PlayerController::IsInMindControl()
{
	return mRemoteAbilityMindControl && mRemoteAbilityMindControl->IsActiveState();
}

void Script_PlayerController::UpdateUI()
{
	mChatBoxUI->Update();
	mPheroBarUI->Update(mScript->GetCurPheroAmount());
	mHpBarUI->Update(mScript->GetCrntHp());

	if (mHurtUIdelta > 0.f) {
		mHurtUIdelta -= DeltaTime() * 0.5f;
		if (mHurtUIdelta <= 0.f) {
			mHurtUIdelta = 0.f;
		}
		mHurtUI->SetOpacity(mHurtUIdelta);
	}

	if (mAimHitUI->IsActive()) {
		mAimHitUI->SetPosition(InputMgr::I->GetMousePos());
		mHitUIDelta -= DeltaTime() * 1.3f;
		if (mHitUIDelta < 0) {
			mHitUIDelta = 0.f;
			mAimHitUI->SetActive(false);
		}
		
		if (mHitUIDelta > 0.8f) {
			float scale = 60 * mHitUIDelta;
			mAimHitUI->SetScale(Vec2(scale, scale));
		}
		else {
			mAimHitUI->SetScale(Vec2(45, 45));
		}
		mAimHitUI->SetOpacity(mHitUIDelta);
	}
}
