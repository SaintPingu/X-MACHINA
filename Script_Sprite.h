#pragma once
#include "Script_Billboard.h"

class Script_Sprite : public Script_Billboard {
	COMPONENT(Script_Billboard, Script_Sprite)

private:
	bool mIsEnd{ false };
	float mRows{};
	float mRow{};
	float mCols{};
	float mCol{};
	
	float mSpeed{ 0.01f };
	float mElapsedTime{};

	Vec4x4 mTextureMtx{ Matrix4x4::Identity() };
	float mScale{ 1.f };

public:
	virtual void Start() override;
	virtual void Update() override;

	bool IsEnd() const { return mIsEnd; }
	void SetSpeed(float speed) { mSpeed = speed; }
	void SetScale(float scale) { mScale = scale; }

	void UpdateSpriteVariable() const;
};