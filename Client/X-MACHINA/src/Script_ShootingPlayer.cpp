#include "stdafx.h"
#include "Script_Player.h"

#include "Script_Weapon.h"


void Script_ShootingPlayer::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	if (!mWeaponScript) {
		return;
	}

	switch (messageID) {
	case WM_LBUTTONDOWN:
		mWeaponScript->StartFire();
		break;

	case WM_LBUTTONUP:
		mWeaponScript->StopFire();
		break;

	default:
		break;
	}
}