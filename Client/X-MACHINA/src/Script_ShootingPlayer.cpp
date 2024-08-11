#include "stdafx.h"
#include "Script_Player.h"

#include "Script_Weapon.h"
#include "Script_Item.h"
#include "Script_BattleManager.h"
#include "Script_BattleUI.h"

#include "BattleScene.h"
#include "Object.h"

#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"



void Script_ShootingPlayer::Start()
{
	base::Start();

	mBattleUI = BattleScene::I->GetManager()->GetComponent<Script_BattleManager>()->GetUI();
	mBattleUI->CreatePlayerUI(this);
}

void Script_ShootingPlayer::OnDestroy()
{
	base::OnDestroy();

	for (auto& weapon : mWeapons) {
		if (weapon) {
			weapon->mParent = nullptr;
			BattleScene::I->RemoveDynamicObject(weapon);
		}
	}

	mBattleUI->RemovePlayer(mObject);
}

void Script_ShootingPlayer::BulletFired()
{
	UpdateWeaponUI();
}

bool Script_ShootingPlayer::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	if (!base::ProcessMouseMsg(messageID, wParam, lParam)) {
		return false;
	}

	switch (messageID) {
	case WM_LBUTTONDOWN:
		StartFire();
		break;

	case WM_LBUTTONUP:
		StopFire();
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
			// weapons //
		case 'R':
			Reload();
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		{
			const int weaponNum = static_cast<int>(wParam - '0');
			DrawWeapon(weaponNum);
		}
		break;
		}
	}
	break;
	}

	return true;
}

void Script_ShootingPlayer::StartFire()
{
	if (mWeaponScript) {
		mWeaponScript->StartFire();
	}
}

void Script_ShootingPlayer::StopFire()
{
	if (mWeaponScript) {
		mWeaponScript->StopFire();
	}
}

bool Script_ShootingPlayer::Reload()
{
	if (IsInGunChangeMotion()) {
		return false;
	}

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

		if (mBattleUI) {
			mBattleUI->SetWeapon(mObject, mWeaponScript);
		}

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

void Script_ShootingPlayer::RemoveWeaponUI() const
{
	if (mBattleUI) {
		mBattleUI->SetWeapon(mObject, nullptr);
	}
}

void Script_ShootingPlayer::UpdateWeaponUI() const
{
	if (mBattleUI) {
		mBattleUI->UpdateWeapon(mObject);
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
		weapon->SetLocalRotation(Vec3(0, 90, 0));
		weapon->SetTag(ObjectTag::Item);

		const auto& weaponItem = weapon->AddComponent<Script_Item_Weapon>();
		weaponItem->StartDrop();
		weapon->SetActive(true);

		if (weapon == mWeapon) {
			mWeapon = nullptr;
			mWeaponScript->StopFire();
			mWeaponScript = nullptr;
			mMuzzle = nullptr;
			mCrntWeaponNum = 0;

			RemoveWeaponUI();
		}
		weapon = nullptr;

		mIsInDraw = false;
		mIsInPutback = false;
	}
}