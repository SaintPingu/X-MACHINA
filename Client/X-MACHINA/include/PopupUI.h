#pragma once

class TextBox;
class UI;

class PopupUI {
private:
	UI*					mBackground{};
	UI*					mPopupBox{};
	std::vector<UI*>	mUIs{};
	
public:
	PopupUI(const std::string& popupUI);
	
public:
	void SetActive(bool val);
	void Remove();
	void AddUI(UI* ui) { mUIs.push_back(ui); }
};