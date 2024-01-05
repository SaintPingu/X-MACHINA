#pragma once


#pragma region Include
#include "Script_Billboard.h"
#pragma endregion


#pragma region Class
class Script_Sprite : public Script_Billboard {
	COMPONENT(Script_Sprite, Script_Billboard)

private:
	bool  mIsEndAnimation{ false };		// �ִϸ��̼��� ����Ǿ��°�? (������ �����ӿ� �����ߴ°�?)

	int mRows{};	// �� ����
	int mRow{};		// ���� ��
	int mCols{};	// �� ����
	int mCol{};		// ���� ��
	
	float mSpeed{ 0.01f };	// �ִϸ��̼� �ӵ�
	float mElapsedTime{};	// �ִϸ��̼� ���� ����ð�

	Vec4x4 mTextureMtx{ Matrix4x4::Identity() };

public:
	bool IsEndAnimation() const { return mIsEndAnimation; }

	void SetSpeed(float speed) { mSpeed = speed; }

public:
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual void UpdateSpriteVariable() const override;
};
#pragma endregion