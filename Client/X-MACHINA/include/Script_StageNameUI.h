#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion

class UI;
class GameObject;

#pragma region Class
class Script_StageNameUI : public Component {
	COMPONENT(Script_StageNameUI, Component)

private:
	struct StageUI {
		int mTriggerX{};
		UI* mUI{};
	};

private:
	float mT{};
	float mOpacity{};
	int mCrntStageIdx{};
	StageUI* mStageUI{};
	GameObject* mPlayer{};
	std::vector<StageUI> mStageUIs{};

public:
	virtual void Awake() override;
	virtual void Update() override;

private:
	void CheckUIActive();
	void MoveToNextStage();
};
#pragma endregion
