#include "stdafx.h"
#include "Script_Player.h"

#include "Script_Weapon.h"
#include "Script_Item.h"

#include "Object.h"
#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"



void Script_ShootingPlayer::OnDestroy()
{
	base::OnDestroy();

	for (auto& weapon : mWeapons) {
		if (weapon) {
			weapon->mParent = nullptr;
			BattleScene::I->RemoveDynamicObject(weapon);
		}
	}
}

bool Script_ShootingPlayer::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	if (!base::ProcessMouseMsg(messageID, wParam, lParam)) {
		return false;
	}

	if (!mWeaponScript) {
		return true;
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

	return true;
}

bool Script_ShootingPlayer::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	if (!base::ProcessKeyboardMsg(messageID, wParam, lParam)) {
		return false;
	}

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

	return true;
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

void Script_ShootingPlayer::SetWeapon(int weaponNum)
{
	mCrntWeaponNum = weaponNum;
	mWeapon = mWeapons[weaponNum - 1];
	if (mWeapon) {
		mWeapon->SetActive(true);
		mWeaponScript = mWeapon->GetComponent<Script_Weapon>();
		mMuzzle = mWeaponScript->GetMuzzle();

#ifdef SERVER_COMMUNICATION
		auto cpkt = FBS_FACTORY->CPkt_Player_Weapon(mWeaponScript->GetWeaponName());
		CLIENT_NETWORK->Send(cpkt); 
#endif
	}
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
	if (mNextWeaponNum <= 0 || mNextWeaponNum > mWeapons.size()) {
		mCrntWeaponNum = 0;
		DrawWeaponEnd();
		ResetWeaponAnimation();
		return;
	}

	SetWeapon(mNextWeaponNum);
}

void Script_ShootingPlayer::DrawWeaponEnd()
{
	if (mCrntWeaponNum != mNextWeaponNum) {
		SetWeapon(mNextWeaponNum);
	}
	ResetNextWeaponNum();
	mIsInDraw = false;
}

void Script_ShootingPlayer::PutbackWeapon()
{
	ResetNextWeaponNum();
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

	if (mNextWeaponNum != 0) {
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