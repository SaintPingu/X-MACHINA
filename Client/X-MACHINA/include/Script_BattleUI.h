#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region Class
class PlayerUI;
class Script_Weapon;
class Script_ShootingPlayer;
class Script_NetworkRemotePlayer;
class Object;

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
	static constexpr int mkMaxIdx = 3;
	int mLastIdx = -1;

	std::map<const Object*, int> mPlayerIndices{};
	std::vector<sptr<PlayerUI>> mPlayerUIs{};

public:
	virtual void Awake() override;

public:
	void RemovePlayer(const Object* player);
	void UpdateWeapon(const Object* player) const;
	void SetWeapon(const Object* player, rsptr<Script_Weapon> weapon) const;
	void CreatePlayerUI(const Script_ShootingPlayer* player);
	void CreatePlayerUI(const Script_NetworkRemotePlayer* player);

private:
	void CreatePlayerUI(const Object* player, const std::wstring& playerName, int playerLevel);
	bool IsValidPlayer(const Object* player) const { return mPlayerIndices.count(player); }

	Vec2 GetPos(int idx) const;
	const Vec3& GetColor(int idx) const { return mkUIColors.at(idx); }
	int GetIdx(const Object* player) const { return mPlayerIndices.at(player); }
};

#pragma endregion