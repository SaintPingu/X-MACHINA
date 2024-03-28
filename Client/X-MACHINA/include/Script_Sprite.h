#pragma once


#pragma region Include
#include "Script_Billboard.h"
#pragma endregion


#pragma region Class
class Script_Sprite : public Script_Billboard {
	COMPONENT(Script_Sprite, Script_Billboard)

private:
	bool  mIsEndAnimation{ false };		// 애니메이션이 종료되었는가? (마지막 프레임에 도달했는가?)

	int mRows{};	// 행 개수
	int mRow{};		// 현재 행
	int mCols{};	// 열 개수
	int mCol{};		// 현재 열
	
	float mSpeed{ 0.01f };	// 애니메이션 속도
	float mElapsedTime{};	// 애니메이션 실행 경과시간

	Matrix mTextureMtx{ };

public:
	bool IsEndAnimation() const { return mIsEndAnimation; }

	void SetSpeed(float speed) { mSpeed = speed; }

public:
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual void UpdateSpriteVariable(const int matIndex) const override;
};
#pragma endregion