#include "EnginePch.h"
#include "Bloom.h"

#include "Texture.h"
#include "Mesh.h"
#include "Shader.h"
#include "DXGIMgr.h"
#include "ResourceMgr.h"
#include "MultipleRenderTarget.h"

Bloom::Bloom()
{
	for (int i = 0; i < BloomCount; ++i) {
		mTargets[i] = DXGIMgr::I->GetMRT(GroupType::Bloom)->GetTexture(i);
	}

	mOutput = mTargets.back();
}

void Bloom::Execute()
{
	for (int i = 0; i < BloomCount; ++i) {
		if (i > static_cast<int>(BloomType::Luminance)) {
			const int prevTarget = mTargets[i - 1]->GetSrvIdx();
			DXGIMgr::I->SetGraphicsRoot32BitConstants(RootParam::Bloom, prevTarget, 0);
		}

		DXGIMgr::I->GetMRT(GroupType::Bloom)->ClearRenderTargetView(i, 0.f);
		DXGIMgr::I->GetMRT(GroupType::Bloom)->OMSetRenderTargets(1, i);
		
		switch (static_cast<BloomType>(i))
		{
		case BloomType::Luminance:
			ExtractLuminace();
			break;
		case BloomType::DownSampling0:
		case BloomType::DownSampling1:
			mBaseTargets.push(mTargets[i]);
		case BloomType::DownSampling2:
			DownSampling();
			break;
		case BloomType::UpSampling0:
		case BloomType::UpSampling1:
			UpSampling();
			break;
		case BloomType::Blur0_h:
		case BloomType::Blur0_v:
		case BloomType::Blur1_h:
		case BloomType::Blur1_v:
		case BloomType::Blur2_h:
		case BloomType::Blur2_v:
			Blur(i % 2, Vec2{ mTargets[i - 1]->GetWidth(), mTargets[i - 1]->GetHeight() });
			break;
		}

		DXGIMgr::I->GetMRT(GroupType::Bloom)->WaitTargetToResource(i);
	}
}

UINT Bloom::GetOutput()
{
	return mOutput->GetSrvIdx();
}

void Bloom::ExtractLuminace()
{
	RESOURCE<Shader>("Luminance")->Set();

	RESOURCE<ModelObjectMesh>("Rect")->Render();
}

void Bloom::DownSampling()
{
	RESOURCE<Shader>("DownSampling")->Set();

	RESOURCE<ModelObjectMesh>("Rect")->Render();
}

void Bloom::UpSampling()
{
	DXGIMgr::I->SetGraphicsRoot32BitConstants(RootParam::Bloom, static_cast<int>(mBaseTargets.top()->GetSrvIdx()), 1);
	mBaseTargets.pop();

	RESOURCE<Shader>("UpSampling")->Set();

	RESOURCE<ModelObjectMesh>("Rect")->Render();
}

void Bloom::Blur(int vertBlur, const Vec2& renderTargetSize)
{
	DXGIMgr::I->SetGraphicsRoot32BitConstants(RootParam::Bloom, 1.f / renderTargetSize.x, 2);
	DXGIMgr::I->SetGraphicsRoot32BitConstants(RootParam::Bloom, 1.f / renderTargetSize.y, 3);
	DXGIMgr::I->SetGraphicsRoot32BitConstants(RootParam::Bloom, vertBlur, 4);

	RESOURCE<Shader>("BloomBlur")->Set();

	RESOURCE<ModelObjectMesh>("Rect")->Render();
}
