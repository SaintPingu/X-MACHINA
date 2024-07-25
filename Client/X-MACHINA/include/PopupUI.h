#pragma once

class TextBox;
class UI;

class PopupUI {
private:
	UI*				mBackground{};
	UI*				mPopupBox{};
	
public:
	PopupUI();
	
public:
	void SetActive(bool val);
	void Remove();
};