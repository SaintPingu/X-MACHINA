#include "EnginePch.h"
#include "Ssao.h"
#include "Texture.h"
#include "ResourceMgr.h"
#include "DXGIMgr.h"
#include "Shader.h"
#include "Mesh.h"
#include "MultipleRenderTarget.h"

Ssao::Ssao(ID3D12GraphicsCommandList* directCmdList)
{
	// ssao map의 멀티 렌더 타겟을 가져온다.
	mSsao0Map = DXGIMgr::I->GetMRT(GroupType::Ssao)->GetTexture(SsaoMap::Ssao0);
	mSsao1Map = DXGIMgr::I->GetMRT(GroupType::Ssao)->GetTexture(SsaoMap::Ssao1);

	// 랜덤 벡터 텍스처와 오프셋 벡터를 생성한다.
	CreateRandomVectorTexture(directCmdList);
	CreateOffsetVectors();
}

void Ssao::GetOffsetVectors(Vec4 offsets[14])
{
	std::copy(&mOffsets[0], &mOffsets[14], &offsets[0]);
}

void Ssao::Execute(int blurCount)
{
	// ssao map을 렌더링한다.
	RenderSsaoMap();
	// ssao map을 흐린다.
	BlurSsaoMap(blurCount);
}

void Ssao::RenderSsaoMap()
{
	RESOURCE<Shader>("Ssao")->Set();

	// ssao 렌더 타겟 0번에 차폐정도를 렌더링한다.
	DXGIMgr::I->GetMRT(GroupType::Ssao)->ClearRenderTargetView(0, 1.f);
	DXGIMgr::I->GetMRT(GroupType::Ssao)->OMSetRenderTargets(1, 0);

	RESOURCE<ModelObjectMesh>("Rect")->Render();

	DXGIMgr::I->GetMRT(GroupType::Ssao)->WaitTargetToResource(0);
}

void Ssao::BlurSsaoMap(int blurCount)
{
	RESOURCE<Shader>("SsaoBlur")->Set();

	// 수평, 수직으로 ssao map을 흐린다.
	for (int i = 0; i < blurCount; ++i) {
		BlurSsaoMap(true);
		BlurSsaoMap(false);
	}
}

void Ssao::BlurSsaoMap(bool horzBlur)
{
	int inputMapHandleIndex{};	// 흐릴 대상 ssao map 렌더 타겟 텍스처 인덱스
	int outputMapIndex{};		// 흐린 결과 ssao map 렌더 타겟 텍스처 인덱스

	// 수평 흐리기인 경우 ssao 렌더 타겟의 0번 텍스처를 인풋으로 설정
	// 수직 흐리기인 경우 ssao 렌더 타겟의 1번 텍스처를 인풋으로 설정
	if (horzBlur) {
		inputMapHandleIndex = mSsao0Map->GetSrvIdx();
		outputMapIndex = static_cast<UINT8>(SsaoMap::Ssao1);
	}
	else {
		inputMapHandleIndex = mSsao1Map->GetSrvIdx();
		outputMapIndex = static_cast<UINT8>(SsaoMap::Ssao0);
	}

	// 인덱스에 따라서 루트 상수 연결
	DXGIMgr::I->SetGraphicsRoot32BitConstants(RootParam::SsaoBlur, inputMapHandleIndex, 0);
	DXGIMgr::I->SetGraphicsRoot32BitConstants(RootParam::SsaoBlur, outputMapIndex, 1);

	// blur ssao map
	DXGIMgr::I->GetMRT(GroupType::Ssao)->ClearRenderTargetView(outputMapIndex, 1.f);
	DXGIMgr::I->GetMRT(GroupType::Ssao)->OMSetRenderTargets(1, outputMapIndex);

	RESOURCE<ModelObjectMesh>("Rect")->Render();

	DXGIMgr::I->GetMRT(GroupType::Ssao)->WaitTargetToResource(outputMapIndex);
}

void Ssao::CreateRandomVectorTexture(ID3D12GraphicsCommandList* directCmdList)
{
	constexpr int kTexSize = 256;

	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = kTexSize;
	texDesc.Height = kTexSize;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	DEVICE->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mRandomVecMap));

	const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(mRandomVecMap.Get(), 0, num2DSubresources);

	DEVICE->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(mRandomVecMapUploadBuffer.GetAddressOf()));

	XMCOLOR initData[kTexSize * kTexSize];
	for (int i = 0; i < kTexSize; ++i) {
		for (int j = 0; j < kTexSize; ++j) {
			// Random vector in [0,1].  We will decompress in shader to [-1,1].
			Vec3 v(Math::RandFloat(), Math::RandFloat(), Math::RandFloat());

			initData[i * kTexSize + j] = XMCOLOR(v.x, v.y, v.z, 0.0f);
		}
	}

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = kTexSize * sizeof(XMCOLOR);
	subResourceData.SlicePitch = subResourceData.RowPitch * kTexSize;

	D3DUtil::ResourceTransition(mRandomVecMap.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
	UpdateSubresources(directCmdList, mRandomVecMap.Get(), mRandomVecMapUploadBuffer.Get(), 0, 0, num2DSubresources, &subResourceData);
	D3DUtil::ResourceTransition(mRandomVecMap.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

	sptr<Texture> mRandomVecTexture = ResourceMgr::I->CreateTexture("RandomVector", mRandomVecMap);
	DXGIMgr::I->CreateShaderResourceView(mRandomVecTexture.get());
}

void Ssao::CreateOffsetVectors()
{
	// 8 cube corners
	mOffsets[0] = Vec4(+1.0f, +1.0f, +1.0f, 0.0f);
	mOffsets[1] = Vec4(-1.0f, -1.0f, -1.0f, 0.0f);
	mOffsets[2] = Vec4(-1.0f, +1.0f, +1.0f, 0.0f);
	mOffsets[3] = Vec4(+1.0f, -1.0f, -1.0f, 0.0f);
	mOffsets[4] = Vec4(+1.0f, +1.0f, -1.0f, 0.0f);
	mOffsets[5] = Vec4(-1.0f, -1.0f, +1.0f, 0.0f);
	mOffsets[6] = Vec4(-1.0f, +1.0f, -1.0f, 0.0f);
	mOffsets[7] = Vec4(+1.0f, -1.0f, +1.0f, 0.0f);

	// 6 centers of cube faces
	mOffsets[8] = Vec4(-1.0f, 0.0f, 0.0f, 0.0f);
	mOffsets[9] = Vec4(+1.0f, 0.0f, 0.0f, 0.0f);
	mOffsets[10] = Vec4(0.0f, -1.0f, 0.0f, 0.0f);
	mOffsets[11] = Vec4(0.0f, +1.0f, 0.0f, 0.0f);
	mOffsets[12] = Vec4(0.0f, 0.0f, -1.0f, 0.0f);
	mOffsets[13] = Vec4(0.0f, 0.0f, +1.0f, 0.0f);

	for (int i = 0; i < 14; ++i) {
		// Create random lengths in [0.25, 1.0].
		float s = Math::RandFloat(0.25f, 1.f);
		
		Vec4 v = s * XMVector4Normalize(mOffsets[i]);

		mOffsets[i] = v;
	}
}