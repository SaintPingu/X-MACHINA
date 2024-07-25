#include "stdafx.h"
#include "PopupUI.h"

#include "GameFramework.h"

#include "TextMgr.h"

#include "Component/UI.h"

PopupUI::PopupUI(const std::string& popupUI)
{
	mBackground = Canvas::I->CreateUI<UI>(5, "Black", Vec2::Zero, GameFramework::I->GetWindowSize());
	mBackground->SetOpacity(0.3f);
	mBackground->SetHoverable(true);
	mPopupBox = Canvas::I->CreateUI<UI>(6, "QuitPopup", Vec2::Zero);;
}

void PopupUI::SetActive(bool val)
{
	mBackground->SetActive(val);
	mPopupBox->SetActive(val);

	for (const auto& ui : mUIs) {
		ui->SetActive(val);
	}
}

void PopupUI::Remove()
{
	mBackground->Remove();
	mPopupBox->Remove();
	mBackground = nullptr;
	mPopupBox = nullptr;

	for (const auto& ui : mUIs) {
		ui->Remove();
	}

	mUIs.clear();
}