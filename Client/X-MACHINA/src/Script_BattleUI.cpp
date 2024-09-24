#include "stdafx.h"
#include "Script_BattleUI.h"

#include "Script_Weapon.h"
#include "Script_Player.h"
#include "ClientNetwork/Contents/Script_NetworkRemotePlayer.h"

#include "Object.h"
#include "PlayerUI.h"

void Script_BattleUI::Awake()
{
	base::Awake();

	mPlayerUIs.resize(mkMaxIdx + 1);
}

void Script_BattleUI::Update()
{
	base::Update();

	for (auto& playerUI : mPlayerUIs) {
		if (playerUI) {
			playerUI->UpdateSimple();
		}
	}
}

void Script_BattleUI::RemovePlayer(const Object* player)
{
	if (!IsValidPlayer(player)) {
		return;
	}

	int targetIdx = GetIdx(player);
	mPlayerIndices.erase(player);

	for (int i = targetIdx; i < mkMaxIdx; ++i) {
		if (i == mLastIdx) {
			break;
		}

		mPlayerUIs[i] = mPlayerUIs[i + 1];
		mPlayerUIs[i]->SetPosition(GetPos(i));
		mPlayerUIs[i]->SetColor(GetColor(i));
	}

	for (auto& [p, idx] : mPlayerIndices) {
		if (idx >= targetIdx) {
			--idx;
		}
	}

	mPlayerUIs[mLastIdx] = nullptr;
	--mLastIdx;
}

void Script_BattleUI::UpdateWeapon(const Object* player) const
{
	if (!IsValidPlayer(player)) {
		return;
	}

	mPlayerUIs.at(GetIdx(player))->Update();
}

void Script_BattleUI::SetWeapon(const Object* player, rsptr<Script_Weapon> weapon) const
{
	if (!IsValidPlayer(player)) {
		return;
	}

	mPlayerUIs.at(GetIdx(player))->SetWeapon(weapon);
}

void Script_BattleUI::CreatePlayerUI(const Script_ShootingPlayer* player)
{
	const std::string& playerName = player->GetObj()->GetName();
	const int playerLevel = player->GetLevel();

	CreatePlayerUI(player->GetObj(), playerName, playerLevel);
}

void Script_BattleUI::CreatePlayerUI(const Script_NetworkRemotePlayer* player)
{
	const std::string& playerName = player->GetObj()->GetName();
	const int playerLevel = player->GetLevel();

	CreatePlayerUI(player->GetObj(), playerName, playerLevel);
}

void Script_BattleUI::CreatePlayerUI(const Object* player, const std::string& playerName, int playerLevel)
{
	if (mLastIdx == mkMaxIdx) {
		return;
	}
	for (int i = 0; i < mPlayerUIs.size(); ++i) {
		if (!mPlayerUIs[i]) {
			break;
		}
		if (player == mPlayerUIs[i]->GetPlayer()) {
			return;
		}
	}
	++mLastIdx;

	sptr<PlayerUI> playerUI = std::make_shared<PlayerUI>(GetPos(mLastIdx), GetColor(mLastIdx), player, playerName, playerLevel);
	mPlayerUIs[mLastIdx] = playerUI;

	mPlayerIndices[player] = mLastIdx;
}

Vec2 Script_BattleUI::GetPos(int idx) const
{
	static constexpr Vec2 kStartOffset = Vec2(-450, -330);
	static constexpr Vec2 kGap = Vec2(300, 0);

	return kStartOffset + (kGap * static_cast<float>(idx));
}