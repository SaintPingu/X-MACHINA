#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion

class UI;
class Button;
class PopupUI;
class Script_AimController;
class Script_LobbyManager;

#pragma region Class
class Script_LobbyUI : public Component {
	COMPONENT(Script_LobbyUI, Component)

private:
	Script_LobbyManager* mLobbyManager{};
	sptr<PopupUI> mPopupQuit{};
	Button* mCustomLeftArrow{};
	Button* mCustomRightArrow{};
	bool mIsInCustom{};

public:
	virtual void Awake() override;

public:
	void SetLobbyManager(Script_LobbyManager* manager) { mLobbyManager = manager; }

	bool IsInCustom() const { return mIsInCustom; }

private:
	void PlayButton() const;
	void QuitGame() const;
	void ShowQuitPopup() const;
	void HideQuitPopup() const;

	void CustomButton();
	void CustomLeftArrowButton();
	void CustomRightArrowButton();
};

#pragma endregion