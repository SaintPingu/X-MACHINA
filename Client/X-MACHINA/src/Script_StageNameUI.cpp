#include "stdafx.h"
#include "Script_StageNameUI.h"

#include "Timer.h"
#include "Object.h"
#include "GameFramework.h"

#include "Component/UI.h"

void Script_StageNameUI::Awake()
{
	base::Awake();

	mPlayer = GameFramework::I->GetPlayer();
	mT = 0.f;

	Vec2 pos(5, 350);
	mStageUIs.emplace_back(0, Canvas::I->CreateUI<UI>(9, "BaseCampName", pos));
	mStageUIs.emplace_back(110, Canvas::I->CreateUI<UI>(9, "Stage1Name", pos));
	mStageUIs.emplace_back(240, Canvas::I->CreateUI<UI>(9, "UrsacetusName", pos));
	mStageUIs.emplace_back(300, Canvas::I->CreateUI<UI>(9, "Stage2Name", pos));
	mStageUIs.emplace_back(440, Canvas::I->CreateUI<UI>(9, "TutelarisName", pos));
	mStageUIs.emplace_back(510, Canvas::I->CreateUI<UI>(9, "Stage3Name", pos));
	mStageUIs.emplace_back(620, Canvas::I->CreateUI<UI>(9, "DeusName", pos));

	for (const auto& ui : mStageUIs) {
		ui.mUI->SetActive(false);
	}
}

void Script_StageNameUI::Update()
{
	base::Update();

	if (!mStageUI) {
		CheckUIActive();
		return;
	}

	if (!mStageUI->mUI) {
		++mCrntStageIdx;
		return;
	}

	mT += DeltaTime() * 0.65f;
	mOpacity = sin(mT);
	mStageUI->mUI->SetOpacity(mOpacity);

	if (mT >= XM_PI) {
		MoveToNextStage();
	}
}

void Script_StageNameUI::CheckUIActive()
{
	if (mCrntStageIdx >= mStageUIs.size()) {
		return;
	}

	auto& ui = mStageUIs[mCrntStageIdx];
	if (mPlayer->GetPosition().x >= ui.mTriggerX) {
		mStageUI = &ui;
		mStageUI->mUI->SetOpacity(0.f);
		mStageUI->mUI->SetActive(true);
	}
}

void Script_StageNameUI::MoveToNextStage()
{
	mOpacity = 0.f;
	mT = 0.f;
	mStageUI->mUI->Remove();
	mStageUI->mUI = nullptr;
	mStageUI = nullptr;
	++mCrntStageIdx;
}
