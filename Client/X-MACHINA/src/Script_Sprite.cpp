#include "stdafx.h"
#include "Script_Sprite.h"
#include "DXGIMgr.h"

#include "Timer.h"
#include "Object.h"
#include "Texture.h"
#include "Scene.h"

#include "FrameResource.h"

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
	ObjectConstants objectConstants;
	objectConstants.MtxWorld = XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(mScale, mScale, 1.f), _MATRIX(mObject->GetWorldTransform())));
	objectConstants.MtxSprite = XMMatrix::Transpose(mTextureMtx);

	// 현재 물고 있는 오브젝트의 상수 버퍼 인덱스를 복사한다.
	int objCBIdx = mObject->GetObjCBIdx();

	// 상수 버퍼에 매핑하고 objCBIdx에 사용할 상수 버퍼 인덱스를 저장한다.
	frmResMgr->CopyData(objCBIdx, objectConstants);

	// 현재 오브젝트의 상수 버퍼 인덱스가 설정되지 않은 경우에만 새롭게 설정한다.
	if (!mObject->GetUseObjCB()) {
		mObject->SetUseObjCB(true);
		mObject->SetObjCBIdx(objCBIdx);
	}

	// 상수 버퍼 뷰 Set
	scene->SetGraphicsRootConstantBufferView(RootParam::Object, frmResMgr->GetObjCBGpuAddr(objCBIdx));
}