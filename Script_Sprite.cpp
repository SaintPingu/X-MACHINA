#include "stdafx.h"
#include "Script_Sprite.h"
#include "DXGIMgr.h"

#include "Timer.h"
#include "Object.h"
#include "Texture.h"
#include "Collider.h"
#include "GameFramework.h"
#include "Scene.h"

namespace {
	struct SpriteInfo {
		int rows{};
		int cols{};
	};
}

void Script_Sprite::Start()
{
	const std::unordered_map<std::string, SpriteInfo> spriteMap{
		{"Explode_8x8", {8, 8}},
	};

	base::Start();

	GameObject* gameObject = mObject->GetObj<GameObject>();

	rsptr<Texture> texture = gameObject->GetTexture();
	SpriteInfo info        = spriteMap.at(texture->GetName());

	mRows = info.rows;
	mCols = info.cols;
}

void Script_Sprite::Update()
{
	base::Update();

	mElapsedTime += DeltaTime() * 0.5f;
	if (mElapsedTime >= mSpeed) {
		mElapsedTime = 0.f;
	}

	mTextureMtx._11 = 1.f / mRows;
	mTextureMtx._22 = 1.f / mCols;
	mTextureMtx._31 = (float)mCol / mCols;
	mTextureMtx._32 = (float)mRow / mRows;

	if (mElapsedTime == 0.f)
	{
		if (++mCol >= mCols) { mRow++; mCol = 0; }
		if (mRow == mRows) {
			mRow = 0;
			mIsEndAnimation = true;
		}
	}
}

void Script_Sprite::UpdateSpriteVariable() const
{
	base::UpdateSpriteVariable();

	scene->SetGraphicsRoot32BitConstants(RootParam::SpriteInfo, XMMatrix::Transpose(mTextureMtx), 0);
}