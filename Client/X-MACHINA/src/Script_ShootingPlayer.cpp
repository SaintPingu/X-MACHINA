#include "stdafx.h"
#include "Script_Player.h"

#include "Script_Weapon.h"

#include "Object.h"


void Script_ShootingPlayer::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	if (!mWeaponScript) {
		return;
	}

	switch (messageID) {
	case WM_LBUTTONDOWN:
		StartFire();
		break;

	case WM_LBUTTONUP:
		StopFire();
		break;

	default:
		break;
	}
}

void Script_ShootingPlayer::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	switch (messageID) {
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case 'R':
			if (!IsInGunChangeMotion()) {
				Reload();
			}
			break;

		default:
			break;
		}
	}
	break;
	default:
		break;
	}
}

void Script_ShootingPlayer::StartFire()
{
	mWeaponScript->StartFire();
}

void Script_ShootingPlayer::StopFire()
{
	mWeaponScript->StopFire();
}

void Script_ShootingPlayer::Reload()
{
	if (mWeaponScript) {
		mWeaponScript->InitReload();
	}
}

void Script_ShootingPlayer::SetWeapon(int weaponIdx)
{
	if (IsInGunChangeMotion()) {
		return;
	}

	if (weaponIdx == 0) {
		if (mWeapon) {
			PutbackWeapon();
		}
		return;
	}

	// 이미 무기를 들고 있다면 putback 후 draw한다.
	if (mWeapon) {
		if (mWeapon == mWeapons[weaponIdx - 1]) {
			return;
		}
		else {
			PutbackWeapon();
		}

		mNextWeaponIdx = weaponIdx;
	}
	else {
		DrawWeaponStart(weaponIdx, false);
	}
}

void Script_ShootingPlayer::DrawWeaponStart(int weaponIdx, bool isDrawImmed)
{
	mIsInDraw = true;
	mNextWeaponIdx = weaponIdx;
}

void Script_ShootingPlayer::DrawWeapon()
{
	mCrntWeaponIdx = mNextWeaponIdx;
	mWeapon = mWeapons[mNextWeaponIdx - 1];
	if (mWeapon) {
		mWeapon->OnEnable();
		mWeaponScript = mWeapon->GetComponent<Script_Weapon>();
		mMuzzle = mWeaponScript->GetMuzzle();
	}
}

void Script_ShootingPlayer::DrawWeaponEnd()
{
	mNextWeaponIdx = -1;
	mIsInDraw = false;
}

void Script_ShootingPlayer::PutbackWeapon()
{
	mNextWeaponIdx = -1;
	mIsInPutback = true;

	if (mWeaponScript) {
		StopFire();
	}
}

void Script_ShootingPlayer::PutbackWeaponEnd()
{
	mIsInPutback = false;

	if (mWeapon) {
		mWeapon->OnDisable();
		mWeapon = nullptr;
		mWeaponScript = nullptr;
		mMuzzle = nullptr;
	}

	if (mNextWeaponIdx != -1) {
		// putback이 끝난 후 바로 draw
		DrawWeaponStart(mNextWeaponIdx, true);
	}

	mCrntWeaponIdx = 0;
}
