#include "stdafx.h"
#include "Script_BattleUI.h"

#include "Script_Weapon.h"
#include "Script_Player.h"

#include "PlayerUI.h"
#include "InputMgr.h"

void Script_BattleUI::Awake()
{
	base::Awake();
}

void Script_BattleUI::RemovePlayer(const Script_ShootingPlayer* player)
{
	if (!IsValidPlayer(player)) {
		return;
	}

}

void Script_BattleUI::UpdateWeapon(const Script_ShootingPlayer* player) const
{
	if (!IsValidPlayer(player)) {
		return;
	}

	mPlayerUIs.at(mPlayers.at(player))->Update();
}

void Script_BattleUI::SetWeapon(const Script_ShootingPlayer* player) const
{
	if (!IsValidPlayer(player)) {
		return;
	}

	mPlayerUIs.at(mPlayers.at(player))->SetWeapon(player->GetCrntWeapon());
}

void Script_BattleUI::CreatePlayerUI(const Script_ShootingPlayer* player)
{
	static constexpr Vec2 kStartOffset = Vec2(-450, -350);
	static constexpr Vec2 kGap = Vec2(300, 0);

	if (!player) {
		return;
	}

	const int idx = static_cast<int>(mPlayerUIs.size());
	const Vec2 pos = kStartOffset + (kGap * static_cast<float>(idx));
	const std::wstring& playerName = player->GetName();
	const int playerLevel = player->GetLevel();

	sptr<PlayerUI> playerUI = std::make_shared<PlayerUI>(pos, mkUIColors[idx], playerName, playerLevel);
	mPlayerUIs.push_back(playerUI);

	mPlayers[player] = idx;
}
