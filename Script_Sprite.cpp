#include "stdafx.h"
#include "Script_Sprite.h"
#include "DXGIMgr.h"

#include "Timer.h"
#include "Object.h"
#include "Texture.h"
#include "Collider.h"
#include "GameFramework.h"
#include "Scene.h"

struct SpriteInfo {
	int rows{};
	int cols{};
};

void Script_Sprite::Start()
{
	const std::unordered_map<std::wstring, SpriteInfo> spriteMap{
		{L"Explode_8x8", {8, 8}},
	};

	base::Start();

	CGameObject* gameObject = mObject->Object<CGameObject>();
	gameObject->SetFlyable(true);

	rsptr<CTexture> texture = gameObject->GetTexture();
	SpriteInfo info = spriteMap.at(texture->GetName());
	mRows = info.rows;
	mCols = info.cols;
}

void Script_Sprite::Update()
{
	base::Update();

	mElapsedTime += DeltaTime() * 0.5f;
	if (mElapsedTime >= mSpeed) {
		mElapsedTime = 0.0f;
	}

	mTextureMtx._11 = 1.0f / mRows;
	mTextureMtx._22 = 1.0f / mCols;
	mTextureMtx._31 = mCol / mCols;
	mTextureMtx._32 = mRow / mRows;

	if (mElapsedTime == 0.0f)
	{
		if (++mCol >= mCols) { mRow++; mCol = 0; }
		if (mRow == mRows) {
			mRow = 0;
			mIsEnd = true;
		}
	}
}

void Script_Sprite::UpdateSpriteVariable() const
{
	Matrix scaling = XMMatrixMultiply(XMMatrixScaling(mScale, mScale, 1.f), _MATRIX(mObject->GetWorldTransform()));
	crntScene->SetGraphicsRoot32BitConstants(RootParam::GameObjectInfo, XMMatrixTranspose(scaling), 0);

	crntScene->SetGraphicsRoot32BitConstants(RootParam::SpriteInfo, XMMatrix::Transpose(mTextureMtx), 0);
}