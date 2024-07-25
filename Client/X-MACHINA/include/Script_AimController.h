#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion

#pragma region ClassForwardDecl
class UI;
class TextBox;
class Texture;
#pragma endregion

#pragma region Class
class Script_AimController : public Component {
	COMPONENT(Script_AimController, Component)

private:
	UI* mUI{};
	TextBox* mPosText{};

public:
	virtual void Awake() override;
	virtual void Update() override;

	sptr<Texture> GetTexture() const;
	Vec2 GetTextureScale() const;

public:
	void SetUI(UI* ui) { mUI = ui; }
	void ChangeAimTexture(rsptr<Texture> newTexture, const Vec2& scale);

private:
	void UpdatePosText();
};
#pragma endregion