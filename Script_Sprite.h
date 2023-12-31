#pragma once


#pragma region Include
#include "Script_Billboard.h"
#pragma endregion


#pragma region Class
class Script_Sprite : public Script_Billboard {
	COMPONENT(Script_Sprite, Script_Billboard)

private:
	bool  mIsEnd{ false };

	float mRows{};
	float mRow{};
	float mCols{};
	float mCol{};
	
	float mSpeed{ 0.01f };
	float mElapsedTime{};

	float mScale{ 1.f };

	Vec4x4 mTextureMtx{ Matrix4x4::Identity() };

public:
	bool IsEnd() const { return mIsEnd; }
	void SetSpeed(float speed) { mSpeed = speed; }
	void SetScale(float scale) { mScale = scale; }

public:
	virtual void Start() override;
	virtual void Update() override;

public:
	void UpdateSpriteVariable() const;
};
#pragma endregion