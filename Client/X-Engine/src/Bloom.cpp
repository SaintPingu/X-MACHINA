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
	mLuminance = DXGIMgr::I->GetMRT(GroupType::Luminance)->GetTexture(0);

	for (int i = 0; i < mkSamplingCount; ++i) {
		mDownSamples[i] = DXGIMgr::I->GetMRT(GroupType::DownSampling)->GetTexture(i);
		mUpSamples[i] = DXGIMgr::I->GetMRT(GroupType::UpSampling)->GetTexture(i);
	}
}

void Bloom::Execute()
{
	DownSampling();
	UpSampling();
}

void Bloom::DownSampling()
{
	RESOURCE<Shader>("DownSampling")->Set();

	for (int i = 0; i < mkSamplingCount; ++i) {
		if (i == 0) {
			DXGIMgr::I->SetGraphicsRoot32BitConstants(RootParam::Bloom, static_cast<int>(mLuminance->GetSrvIdx()), 0);
		}
		else {
			DXGIMgr::I->SetGraphicsRoot32BitConstants(RootParam::Bloom, static_cast<int>(mDownSamples[i - 1]->GetSrvIdx()), 0);
		}

		DXGIMgr::I->GetMRT(GroupType::DownSampling)->ClearRenderTargetView(i, 0.f);
		DXGIMgr::I->GetMRT(GroupType::DownSampling)->OMSetRenderTargets(1, i);

		RESOURCE<ModelObjectMesh>("Rect")->Render();

		DXGIMgr::I->GetMRT(GroupType::DownSampling)->WaitTargetToResource(i);
	}
}

void Bloom::UpSampling()
{
}
