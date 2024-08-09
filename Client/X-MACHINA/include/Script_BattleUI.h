#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region Class
class PlayerUI;
class Script_Weapon;
class Script_ShootingPlayer;

class Script_BattleUI : public Component {
	COMPONENT(Script_BattleUI, Component)
		
private:
	static constexpr std::array<Vec3, 4> mkUIColors{
		Vec3(0.994160354f, 0.00894771889f, 0.0755915046f),
		Vec3(0.579661787f, 0.785649717f, 0.320677072f),
		Vec3(0.0594013520f, 0.659722447f, 0.532157719f),
		Vec3(0.0305042304f, 0.640245616f, 0.933808744f)
	};

private:
	std::unordered_map<const Script_ShootingPlayer*, int> mPlayers{};
	std::vector<sptr<PlayerUI>> mPlayerUIs{};

public:
	virtual void Awake() override;

public:
	void RemovePlayer(const Script_ShootingPlayer* player);
	void UpdateWeapon(const Script_ShootingPlayer* player) const;
	void SetWeapon(const Script_ShootingPlayer* player) const;
	void CreatePlayerUI(const Script_ShootingPlayer* player);

private:
	bool IsValidPlayer(const Script_ShootingPlayer* player) const { return mPlayers.count(player); }
};

#pragma endregion