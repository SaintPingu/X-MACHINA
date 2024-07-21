#include "stdafx.h"
#include "Script_Player.h"

#include "Script_Weapon.h"
#include "Script_Item.h"

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
	base::ProcessKeyboardMsg(messageID, wParam, lParam);

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

bool Script_ShootingPlayer::Reload()
{
	if (mWeaponScript) {
		return mWeaponScript->CheckReload();
	}

	return false;
}

void Script_ShootingPlayer::DrawWeapon(int weaponNum)
{
	if (IsInGunChangeMotion()) {
		return;
	}

	if (weaponNum == 0) {
		if (mWeapon) {
			PutbackWeapon();
		}
		return;
	}

	const int weaponIdx = weaponNum - 1;

	if (mWeapons.size() <= weaponIdx) {
		return;
	}
	if (mWeapons[weaponIdx] == nullptr) {
		return;
	}

	// 이미 무기를 들고 있다면 putback 후 draw한다.
	if (mWeapon) {
		if (mWeapon == mWeapons[weaponIdx]) {
			return;
		}
		else {
			PutbackWeapon();
		}

		mNextWeaponNum = weaponNum;
	}
	else {
		DrawWeaponStart(weaponNum, false);
	}
}

void Script_ShootingPlayer::DrawWeaponStart(int weaponNum, bool isDrawImmed)
{
	if (weaponNum <= 0) {
		return;
	}

	mIsInDraw = true;
	mNextWeaponNum = weaponNum;
}

void Script_ShootingPlayer::DrawWeapon()
{
	mCrntWeaponNum = mNextWeaponNum;
	mWeapon = mWeapons[mNextWeaponNum - 1];
	if (mWeapon) {
		mWeapon->SetActive(true);
		mWeaponScript = mWeapon->GetComponent<Script_Weapon>();
		mMuzzle = mWeaponScript->GetMuzzle();
	}
}

void Script_ShootingPlayer::DrawWeaponEnd()
{
	mNextWeaponNum = -1;
		mIsInDraw = false;
}

void Script_ShootingPlayer::PutbackWeapon()
{
	mNextWeaponNum = -1;
	mIsInPutback = true;

	if (mWeaponScript) {
		StopFire();
	}
}

void Script_ShootingPlayer::PutbackWeaponEnd()
{
	mIsInPutback = false;

	if (mWeapon) {
		mWeapon->SetActive(false);
		mWeapon = nullptr;
		mWeaponScript = nullptr;
		mMuzzle = nullptr;
	}

	if (mNextWeaponNum != -1) {
		// putback이 끝난 후 바로 draw
		DrawWeaponStart(mNextWeaponNum, true);
	}

	mCrntWeaponNum = 0;
}

void Script_ShootingPlayer::DropWeapon(int weaponIdx)
{
	if (weaponIdx < 0) {
		return;
	}

	auto& weapon = mWeapons[weaponIdx];
	if (weapon) {
		weapon->DetachParent(false);
		weapon->SetLocalRotation(Quat::Identity);
		weapon->SetTag(ObjectTag::Item);

		const auto& weaponItem = weapon->AddComponent<Script_Item_Weapon>();
		weaponItem->StartDrop();
		weapon->SetActive(true);

		if (weapon == mWeapon) {
			mWeapon = nullptr;
			mWeaponScript = nullptr;
			mMuzzle = nullptr;
			mCrntWeaponNum = 0;
		}
		weapon = nullptr;

		mIsInDraw = false;
		mIsInPutback = false;
	}
}