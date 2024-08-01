#include "stdafx.h"
#include "Script_PlayerController.h"

#include "Script_Player.h"
#include "Script_AbilityHolder.h"

#include "ShieldAbility.h"
#include "IRDetectorAbility.h"
#include "MinimapAbility.h"
#include "MindControlAbility.h"
#include "CloakingAbility.h"


#include "ChatBoxUI.h"

#include "Component/UI.h"

void Script_PlayerController::Awake()
{
	base::Awake();

	mChatBoxUI = std::make_shared<ChatBoxUI>(Vec2{ -750.f, -200.f }, Vec2{ 300.f, 150.f }, "Name");


	mAbilityShield = mObject->AddComponent<Script_CooldownAbilityHolder>();
	mAbilityIRDetector = mObject->AddComponent<Script_AbilityHolder>();
	mAbilityMindControl = mObject->AddComponent<Script_ToggleAbilityHolder>();
	mAbilityCloaking = mObject->AddComponent<Script_ToggleAbilityHolder>();

	mAbilityShield->SetAbility(std::make_shared<ShieldAbility>());
	mAbilityIRDetector->SetAbility(std::make_shared<IRDetectorAbility>());
	mAbilityMindControl->SetAbility(std::make_shared<MindControlAbility>());
	mAbilityCloaking->SetAbility(std::make_shared<CloakingAbility>());
}

void Script_PlayerController::Start()
{
	base::Start();

	mScript = mObject->GetComponent<Script_GroundPlayer>();
}

void Script_PlayerController::Update()
{
	base::Update();

	ProcessInput();

	mChatBoxUI->Update();
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
			break;
		case 'E':
			mScript->Interact();
			break;
		case 'G':
			mScript->DropCrntWeapon();
			break;

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