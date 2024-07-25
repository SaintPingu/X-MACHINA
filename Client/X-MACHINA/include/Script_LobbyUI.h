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
	sptr<Script_AimController> mAimController{};
	UI* mCursorNormal{};
	UI* mCursorClick{};
	sptr<PopupUI> mPopupQuit{};

public:
	virtual void Start() override;
	virtual void Update() override;
	virtual void OnDestroy() override;

private:
	void PlayButton() const;
	void QuitGame() const;
	void ShowQuitPopup() const;
	void HideQuitPopup() const;
};

#pragma endregion