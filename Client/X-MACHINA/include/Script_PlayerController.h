#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


class Script_PheroPlayer;
class Script_Ability;
class Script_AimController;
class ChatBoxUI;
class SliderBarUI;


class Script_PlayerController : public Component {
	COMPONENT(Script_PlayerController, Component)

private:
	sptr<ChatBoxUI> mChatBoxUI{};

	sptr<Script_PheroPlayer>  mScript{};
	sptr<Script_AimController> mAimController{};

	sptr<Script_Ability> mAbilityShield{};
	sptr<Script_Ability> mAbilityIRDetector{};
	sptr<Script_Ability> mAbilityMindControl{};
	sptr<Script_Ability> mAbilityCloaking{};
	sptr<Script_Ability> mAbilityNightVision{};
	sptr<Script_Ability> mAbilityAerialController{};

	sptr<SliderBarUI> mPheroBarUI{};
	sptr<SliderBarUI> mHpBarUI{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

public:
	bool IsActiveChatBox() const;

public:
	virtual bool ProcessInput();
	virtual bool ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
	virtual bool ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam);

public:
	void Chat(const std::string& text);
};
