#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


class Script_GroundPlayer;
class Script_AbilityHolder;
class ChatBoxUI;


class Script_PlayerController : public Component {
	COMPONENT(Script_PlayerController, Component)

private:
	sptr<ChatBoxUI> mChatBoxUI{};

	sptr<Script_GroundPlayer> mScript{};

	sptr<Script_AbilityHolder> mAbilityShield{};
	sptr<Script_AbilityHolder> mAbilityIRDetector{};
	sptr<Script_AbilityHolder> mAbilityMindControl{};
	sptr<Script_AbilityHolder> mAbilityCloaking{};

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
