#include "stdafx.h"
#include "PopupUI.h"

#include "GameFramework.h"

#include "TextMgr.h"

#include "Component/UI.h"

PopupUI::PopupUI()
{
	mBackground = Canvas::I->CreateUI<UI>(0, "Black", Vec2::Zero, GameFramework::I->GetWindowSize());
	mBackground->SetOpacity(0.3f);
}

void PopupUI::SetActive(bool val)
{
	mBackground->SetActive(val);
}

void PopupUI::Remove()
{
	mBackground->Remove();
	mBackground = nullptr;
}