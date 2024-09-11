#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion

class UI;
class PopupUI;
class Script_AimController;

#pragma region Class
class Script_LobbyUI : public Component {
	COMPONENT(Script_LobbyUI, Component)

private:
	sptr<PopupUI> mPopupQuit{};

public:
	virtual void Awake() override;

private:
	void PlayButton() const;
	void QuitGame() const;
	void ShowQuitPopup() const;
	void HideQuitPopup() const;
};

#pragma endregion