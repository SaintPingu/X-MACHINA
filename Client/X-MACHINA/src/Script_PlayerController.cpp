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


#include "ChatBoxUI.h"
#include "SliderBarUI.h"

#include "Component/UI.h"

void Script_PlayerController::Awake()
{
	base::Awake();

	mChatBoxUI = std::make_shared<ChatBoxUI>(Vec2{ -750.f, -200.f }, Vec2{ 300.f, 150.f }, "Name");

	mObject->AddComponent<Script_FootStepSound>();

	const auto& aimUI = Canvas::I->CreateUI<UI>(3, "Aim", Vec2::Zero, Vec2(30, 30));
	mAimController = mObject->AddComponent<Script_AimController>();
	mAimController->SetUI(aimUI);

	mAbilityShield      = mObject->AddComponent<Script_Ability_Shield>(true, false);
	mAbilityIRDetector  = mObject->AddComponent<Script_Ability_IRDetector>(true, false);
	mAbilityMindControl = mObject->AddComponent<Script_Ability_MindControl>(true, false);
	mAbilityCloaking    = mObject->AddComponent<Script_Ability_Cloaking>(true, false);
	mAbilityNightVision = mObject->AddComponent<Script_Ability_NightVision>(true, false);
}

void Script_PlayerController::Start()
{
	base::Start();

	mScript = mObject->GetComponent<Script_PheroPlayer>();

	mHpBarUI = std::make_shared<SliderBarUI>("BackgroundHpBar", "EaseBar", "FillHpBar", Vec2{ 0.f, -425.f }, Vec2{ 1000.f, 15.f }, mScript->GetMaxHp());
	mPheroBarUI = std::make_shared<SliderBarUI>("BackgroundPheroBar", "EaseBar", "FillPheroBar", Vec2{ 0.f, -450.f }, Vec2{ 1000.f, 15.f }, mScript->GetMaxPheroAmount());
}

void Script_PlayerController::Update()
{
	base::Update();

	ProcessInput();

	mChatBoxUI->Update();
	mPheroBarUI->Update(mScript->GetCurPheroAmount());
	mHpBarUI->Update(mScript->GetCrntHp());
}


bool Script_PlayerController::IsActiveChatBox() const
{
	return mChatBoxUI->IsActive();
}

bool Script_PlayerController::ProcessInput()
{
	if (mChatBoxUI->IsActive()) {
		return false;
	}

	mScript->ProcessInput();

	return true;
}

bool Script_PlayerController::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	switch (messageID) {
	case WM_LBUTTONDOWN:
		mScript->StartFire();
		break;

	case WM_LBUTTONUP:
		mScript->StopFire();
		break;

	case WM_RBUTTONDOWN:
		mScript->OnAim();
		break;

	case WM_RBUTTONUP:
		mScript->OffAim();
		break;

	default:
		break;
	}

	return true;
}

bool Script_PlayerController::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	if (!mChatBoxUI) {
		return true;
	}

	switch (messageID) {
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_RETURN:
			mChatBoxUI->ToggleChatBox();
			return false;

			// weapons //
		case 'R':
			mScript->Reload();
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		{
			const int weaponNum = static_cast<int>(wParam - '0');
			mScript->DrawWeapon(weaponNum);
		}
		break;

		case '6':
			mScript->AquireNewWeapon(WeaponName::SkyLine);
			break;
		case '7':
			mScript->AquireNewWeapon(WeaponName::DBMS);
			break;
		case '8':
			mScript->AquireNewWeapon(WeaponName::Burnout);
			break;
		case '9':
			mScript->AquireNewWeapon(WeaponName::PipeLine);

			// items //
			break;
		case 'E':
			mScript->Interact();
			break;
		case 'G':
			mScript->DropCrntWeapon();
			break;

			// phero skills //
		case 'U':
			mAbilityMindControl->Toggle();
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

			// cheats //
		case'L':
			mScript->AddPheroAmount(1000);
			break;
		case'J':
			mScript->Hit(100);
			break;
		case'H':
			mScript->FillHP(1000);
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}


	if (mChatBoxUI->IsActive()) {
		mChatBoxUI->ProcessKeyboardMsg(messageID, wParam, lParam);
		return false;
	}

	return true;
}

void Script_PlayerController::Chat(const std::string& text)
{
	mChatBoxUI->AddChat(text, "other");
}