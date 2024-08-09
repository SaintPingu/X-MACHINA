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
}

void Script_BattleUI::RemovePlayer(const Object* player)
{
	if (!IsValidPlayer(player)) {
		return;
	}

}

void Script_BattleUI::UpdateWeapon(const Object* player) const
{
	if (!IsValidPlayer(player)) {
		return;
	}

	mPlayerUIs.at(mPlayers.at(player))->Update();
}

void Script_BattleUI::SetWeapon(const Object* player, rsptr<Script_Weapon> weapon) const
{
	if (!IsValidPlayer(player)) {
		return;
	}

	mPlayerUIs.at(mPlayers.at(player))->SetWeapon(weapon);
}

void Script_BattleUI::CreatePlayerUI(const Script_ShootingPlayer* player)
{
	const std::wstring& playerName = player->GetName();
	const int playerLevel = player->GetLevel();

	CreatePlayerUI(player->GetObj(), playerName, playerLevel);
}

void Script_BattleUI::CreatePlayerUI(const Script_NetworkRemotePlayer* player)
{
	const std::wstring& playerName = player->GetName();
	const int playerLevel = player->GetLevel();

	CreatePlayerUI(player->GetObj(), playerName, playerLevel);
}

void Script_BattleUI::CreatePlayerUI(const Object* player, const std::wstring& playerName, int playerLevel)
{
	static constexpr Vec2 kStartOffset = Vec2(-450, -350);
	static constexpr Vec2 kGap = Vec2(300, 0);

	const int idx = static_cast<int>(mPlayerUIs.size());
	const Vec2 pos = kStartOffset + (kGap * static_cast<float>(idx));

	sptr<PlayerUI> playerUI = std::make_shared<PlayerUI>(pos, mkUIColors[idx], playerName, playerLevel);
	mPlayerUIs.push_back(playerUI);

	mPlayers[player] = idx;
}