#include "stdafx.h"
#include "DXGIMgr.h"
#include "FrameResource.h"
#include "DescriptorHeap.h"

#include "ResourceMgr.h"
#include "Scene.h"
#include "Shader.h"
#include "Texture.h"
#include "RootSignature.h"
#include "MultipleRenderTarget.h"
#include "BlurFilter.h"
#include "LUTFilter.h"
#include "Ssao.h"
#include "ParticleSystem.h"

#pragma region Imgui
#include "../Imgui/ImguiManager.h"
#pragma endregion


DXGIMgr::DXGIMgr()
	:
	mDescriptorHeap(std::make_shared<DescriptorHeap>())
{
	DWORD filterOptione = 0;
	filterOptione |= FilterOption::None;
	filterOptione |= FilterOption::LUT;
	filterOptione |= FilterOption::Tone;
	filterOptione |= FilterOption::Ssao;
	//filterOptione |= FilterOption::Blur;

	mFilterOption = filterOptione;
}

UINT DXGIMgr::GetGraphicsRootParamIndex(RootParam param) const
{
	return mGraphicsRootSignature->GetRootParamIndex(param);
}

UINT DXGIMgr::GetComputeRootParamIndex(RootParam param) const
{
	return mComputeRootSignature->GetRootParamIndex(param);
}

UINT DXGIMgr::GetParticleComputeRootParamIndex(RootParam param) const
{
	return mParticleComputeRootSignature->GetRootParamIndex(param);
}

RComPtr<ID3D12RootSignature> DXGIMgr::GetGraphicsRootSignature() const
{
	assert(mGraphicsRootSignature);

	return mGraphicsRootSignature->Get();
}

RComPtr<ID3D12RootSignature> DXGIMgr::GetComputeRootSignature() const
{
	assert(mComputeRootSignature);

	return mComputeRootSignature->Get();
}

RComPtr<ID3D12RootSignature> DXGIMgr::GetParticleComputeRootSignature() const
{
	assert(mParticleComputeRootSignature);

	return mParticleComputeRootSignature->Get();
}

void DXGIMgr::SetGraphicsRoot32BitConstants(RootParam param, const Matrix& data, UINT offset)
{
	constexpr UINT kNum32Bit = 16U;
	cmdList->SetGraphicsRoot32BitConstants(GetGraphicsRootParamIndex(param), kNum32Bit, &data, offset);
}

void DXGIMgr::SetGraphicsRoot32BitConstants(RootParam param, const Vec4x4& data, UINT offset)
{
	constexpr UINT kNum32Bit = 16U;
	cmdList->SetGraphicsRoot32BitConstants(GetGraphicsRootParamIndex(param), kNum32Bit, &data, offset);
}

void DXGIMgr::SetGraphicsRoot32BitConstants(RootParam param, const Vec4& data, UINT offset)
{
	constexpr UINT kNum32Bit = 4U;
	cmdList->SetGraphicsRoot32BitConstants(GetGraphicsRootParamIndex(param), kNum32Bit, &data, offset);
}

void DXGIMgr::SetGraphicsRoot32BitConstants(RootParam param, float data, UINT offset)
{
	constexpr UINT kNum32Bit = 1U;
	cmdList->SetGraphicsRoot32BitConstants(GetGraphicsRootParamIndex(param), kNum32Bit, &data, offset);
}

void DXGIMgr::SetGraphicsRoot32BitConstants(RootParam param, int data, UINT offset)
{
	constexpr UINT kNum32Bit = 1U;
	cmdList->SetGraphicsRoot32BitConstants(GetGraphicsRootParamIndex(param), kNum32Bit, &data, offset);
}

void DXGIMgr::SetGraphicsRootConstantBufferView(RootParam param, D3D12_GPU_VIRTUAL_ADDRESS gpuAddr)
{
	cmdList->SetGraphicsRootConstantBufferView(GetGraphicsRootParamIndex(param), gpuAddr);
}

void DXGIMgr::SetGraphicsRootShaderResourceView(RootParam param, D3D12_GPU_VIRTUAL_ADDRESS gpuAddr)
{
	cmdList->SetGraphicsRootShaderResourceView(GetGraphicsRootParamIndex(param), gpuAddr);
}

void DXGIMgr::Init(HINSTANCE hInstance, const WindowInfo& window)
{
	mInstance = hInstance;
	mWindow = window;

	CreateDirect3DDevice();
	CreateCmdQueueAndList();
	CreateSwapChain();
	CreateFrameResources();
	CreateGraphicsRootSignature();
	CreateComputeRootSignature();
	CreateDescriptorHeaps(gkDescHeapCbvCount, gkDescHeapSrvCount, gkDescHeapCbvCount, gkDescHeapSkyBoxCount, gkDescHeapDsvCount);
	CreateDSV();
	CreateMRTs();
	CreateFilter();
	CreateSsao();

	res->LoadResources();
	pr->Init();

	BuildScene();
}

void DXGIMgr::Release()
{
	mGraphicsRootSignature = nullptr;
	mComputeRootSignature = nullptr;
	::CloseHandle(mFenceEvent);
	res->Destroy();
	pr->Destroy();
	Destroy();
}

void DXGIMgr::CreateShaderResourceView(Texture* texture, DXGI_FORMAT srvFormat)
{
	ComPtr<ID3D12Resource> resource = texture->GetResource();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = srvFormat;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.f;

	mDescriptorHeap->CreateShaderResourceView(resource, &srvDesc);
	texture->SetSrvGpuDescriptorHandle(mDescriptorHeap->GetGPUSrvLastHandle(), mDescriptorHeap->GetGPUSrvLastHandleIndex());
}

void DXGIMgr::CreateShaderResourceView(Texture* texture)
{
	ComPtr<ID3D12Resource> resource = texture->GetResource();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = texture->GetShaderResourceViewDesc();

	mDescriptorHeap->CreateShaderResourceView(resource, &srvDesc);

	switch (srvDesc.ViewDimension)
	{
	case D3D12_SRV_DIMENSION_TEXTURE2D:
		texture->SetSrvGpuDescriptorHandle(mDescriptorHeap->GetGPUSrvLastHandle(), mDescriptorHeap->GetGPUSrvLastHandleIndex());
		break;
	case D3D12_SRV_DIMENSION_TEXTURECUBE:
		texture->SetSrvGpuDescriptorHandle(mDescriptorHeap->GetSkyBoxGPUSrvLastHandle(), mDescriptorHeap->GetSkyBoxGPUSrvLastHandleIndex());
		break;
	}
}

void DXGIMgr::CreateUnorderedAccessView(Texture* texture)
{
	ComPtr<ID3D12Resource> resource = texture->GetResource();
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = texture->GetUnorderedAccessViewDesc();

	mDescriptorHeap->CreateUnorderedAccessView(resource, &uavDesc);

	texture->SetUavGpuDescriptorHandle(mDescriptorHeap->GetGPUSrvLastHandle());
}

void DXGIMgr::CreateDepthStencilView(Texture* texture)
{
	ComPtr<ID3D12Resource> resource = texture->GetResource();
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = texture->GetDepthStencilViewDesc();

	mDescriptorHeap->CreateDepthStencilView(resource, &dsvDesc);

	texture->SetDsvGpuDescriptorHandle(mDescriptorHeap->GetCPUDsvLastHandle());
}

void DXGIMgr::Terminate()
{
	BOOL isFullScreenState = FALSE;
	mSwapChain->GetFullscreenState(&isFullScreenState, nullptr);

	if (isFullScreenState) {
		ChangeSwapChainState();
	}
}

void DXGIMgr::Update()
{
	mFrameResourceMgr->Update();
}

void DXGIMgr::Render()
{
	// 현재 프레임의 명령 할당자를 가져온다.
	auto& cmdAllocator = mFrameResourceMgr->GetCurrFrameResource()->CmdAllocator;
	cmdAllocator->Reset();

#pragma region MainRender
	MainPassRenderBegin();

	// 해당 함수들 안에서 자신이 사용할 깊이 버퍼를 클리어 한다.
	GetMRT(GroupType::SwapChain)->ClearRenderTargetView(mCurrBackBufferIdx);
	GetMRT(GroupType::Shadow)->ClearRenderTargetView();
	GetMRT(GroupType::GBuffer)->ClearRenderTargetView();
	GetMRT(GroupType::Lighting)->ClearRenderTargetView();
	GetMRT(GroupType::OffScreen)->ClearRenderTargetView();


	// 그림자 맵 텍스처를 렌더 타겟으로 설정하고 그림자 렌더링
	GetMRT(GroupType::Shadow)->OMSetRenderTargets(0, 0);
	scene->RenderShadow();
	GetMRT(GroupType::Shadow)->WaitTargetToResource();

	// GBuffer를 렌더 타겟으로 설정하고 디퍼드 렌더링
	GetMRT(GroupType::GBuffer)->OMSetRenderTargets();
	scene->RenderDeferred();
	GetMRT(GroupType::GBuffer)->WaitTargetToResource();

	// 라이트 맵 텍스처를 렌더 타겟으로 설정하고 라이트 렌더링
	GetMRT(GroupType::Lighting)->OMSetRenderTargets();
	scene->RenderLights();
	GetMRT(GroupType::Lighting)->WaitTargetToResource();

	// 후면 버퍼대신 화면 밖 텍스처를 렌더 타겟으로 설정하고 렌더링
	GetMRT(GroupType::OffScreen)->OMSetRenderTargets();
	scene->RenderFinal();
	scene->RenderForward();
	GetMRT(GroupType::OffScreen)->WaitTargetToResource();
#pragma endregion

#pragma region PostProcessing
	PostPassRenderBegin();

	UINT offScreenIndex{};
	if (mFilterOption & FilterOption::None)
		offScreenIndex = GetMRT(GroupType::OffScreen)->GetTexture(0)->GetSrvIdx();
	if (mFilterOption & FilterOption::Blur)
		offScreenIndex = mBlurFilter->Execute(GetMRT(GroupType::OffScreen)->GetTexture(0), 4);
	if (mFilterOption & FilterOption::LUT || mFilterOption & FilterOption::Tone)
		offScreenIndex = mLUTFilter->Execute(GetMRT(GroupType::OffScreen)->GetTexture(0));
	if (mFilterOption & FilterOption::Ssao)
		mSsao->Execute(4);

	GetMRT(GroupType::SwapChain)->OMSetRenderTargets(1, mCurrBackBufferIdx);
	scene->RenderPostProcessing(offScreenIndex);
	scene->RenderUI();
	imgui->Render();
	GetMRT(GroupType::SwapChain)->WaitTargetToResource(mCurrBackBufferIdx);
#pragma endregion

	RenderEnd();

	mSwapChain->Present(0, 0);

	MoveToNextFrame();
}

void DXGIMgr::ToggleFullScreen()
{
	ChangeSwapChainState();
}

void DXGIMgr::MainPassRenderBegin()
{
	auto& cmdAllocator = mFrameResourceMgr->GetCurrFrameResource()->CmdAllocator;
	mCmdList->Reset(cmdAllocator.Get(), NULL);

	mDescriptorHeap->Set();

	// 그래픽스 쉐이더 관련 설정
	cmdList->SetGraphicsRootSignature(GetGraphicsRootSignature().Get());
	cmdList->SetGraphicsRootConstantBufferView(GetGraphicsRootParamIndex(RootParam::Ssao), frmResMgr->GetSSAOCBGpuAddr());
	cmdList->SetGraphicsRootShaderResourceView(GetGraphicsRootParamIndex(RootParam::Material), frmResMgr->GetMatBufferGpuAddr());
	cmdList->SetGraphicsRootDescriptorTable(GetGraphicsRootParamIndex(RootParam::Texture), mDescriptorHeap->GetGPUHandle());
	cmdList->SetGraphicsRootDescriptorTable(GetGraphicsRootParamIndex(RootParam::SkyBox), mDescriptorHeap->GetSkyBoxGPUStartSrvHandle());

	// 파티클 컴퓨트 쉐이더 관련 설정
	cmdList->SetComputeRootSignature(GetParticleComputeRootSignature().Get());
	cmdList->SetComputeRootShaderResourceView(GetParticleComputeRootParamIndex(RootParam::ParticleSystem), frmResMgr->GetParticleSystemGpuAddr());
	cmdList->SetComputeRootUnorderedAccessView(GetParticleComputeRootParamIndex(RootParam::ParticleShared), frmResMgr->GetParticleSharedGpuAddr());
}

void DXGIMgr::PostPassRenderBegin()
{
	// imgui 렌더 준비 및 업데이트
	imgui->Render_Prepare();
	imgui->Update();

	// 포스트 프로세싱 컴퓨트 쉐이더 관련 설정
	cmdList->SetComputeRootSignature(GetComputeRootSignature().Get());
}

void DXGIMgr::RenderEnd()
{
	mCmdList->Close();
	ID3D12CommandList* cmdLists[] = { mCmdList.Get() };
	mCmdQueue->ExecuteCommandLists(1, cmdLists);
}

void DXGIMgr::CreateFactory()
{
	HRESULT hResult{};

	UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
	ComPtr<ID3D12Debug> debugController{};
	hResult = ::D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	if (debugController) {
		debugController->EnableDebugLayer();
	}
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mFactory));
}

void DXGIMgr::CreateDevice()
{
	ComPtr<IDXGIAdapter1> adapter{};
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != mFactory->EnumAdapters1(i, &adapter); i++) {
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		adapter->GetDesc1(&dxgiAdapterDesc);

		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			continue;
		}
		else if (SUCCEEDED(::D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice)))) {
			break;
		}
	}

	if (!mDevice) {
		mFactory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		HRESULT hResult = ::D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice));
		AssertHResult(hResult);
	}
}

void DXGIMgr::SetMSAA()
{
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels{};
	msaaQualityLevels.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
	msaaQualityLevels.SampleCount      = 4;
	msaaQualityLevels.Flags            = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msaaQualityLevels.NumQualityLevels = 4;

	HRESULT hResult      = mDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	AssertHResult(hResult);
	mMsaa4xQualityLevels = msaaQualityLevels.NumQualityLevels;
	mIsMsaa4xEnabled     = (mMsaa4xQualityLevels > 1) ? true : false;
}

void DXGIMgr::CreateFence()
{
	HRESULT hResult = mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
	AssertHResult(hResult);
	mFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

void DXGIMgr::SetIncrementSize()
{
	mCbvSrvUavDescriptorIncSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	mRtvDescriptorIncSize		= mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvDescriptorIncSize		= mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void DXGIMgr::CreateDirect3DDevice()
{
	CreateFactory();
	CreateDevice();
	SetMSAA();
	CreateFence();

	SetIncrementSize();
}

void DXGIMgr::CreateCmdQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc{};
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
	HRESULT hResult           = mDevice->CreateCommandQueue(&d3dCommandQueueDesc, IID_PPV_ARGS(&mCmdQueue));
	AssertHResult(hResult);

	hResult = mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCmdAllocator));
	AssertHResult(hResult);
	hResult = mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCmdAllocator.Get(), nullptr, IID_PPV_ARGS(&mCmdList));
	AssertHResult(hResult);
}

void DXGIMgr::CreateSwapChain()
{
	// client의 screen rect를 OS로부터 받아와 설정한다.
	RECT clientRect;
	::GetClientRect(mWindow.Hwnd, &clientRect);
	mWindow.Width = static_cast<short>(clientRect.right - clientRect.left);
	mWindow.Height = static_cast<short>(clientRect.bottom - clientRect.top);

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width              = mWindow.Width;
	swapChainDesc.Height             = mWindow.Height;
	swapChainDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count   = (mIsMsaa4xEnabled) ? 4 : 1;
	swapChainDesc.SampleDesc.Quality = (mIsMsaa4xEnabled) ? (mMsaa4xQualityLevels - 1) : 0;
	swapChainDesc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount        = mSwapChainBuffCnt;
	swapChainDesc.Scaling            = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullScreenDesc{};
	swapChainFullScreenDesc.RefreshRate.Numerator   = 60;
	swapChainFullScreenDesc.RefreshRate.Denominator = 1;
	swapChainFullScreenDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainFullScreenDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainFullScreenDesc.Windowed                = TRUE;

	HRESULT hResult   = mFactory->CreateSwapChainForHwnd(mCmdQueue.Get(), GetHwnd(), &swapChainDesc, &swapChainFullScreenDesc, nullptr, (IDXGISwapChain1**)mSwapChain.GetAddressOf());
	AssertHResult(hResult);

	hResult           = mFactory->MakeWindowAssociation(GetHwnd(), DXGI_MWA_NO_ALT_ENTER);
	AssertHResult(hResult);

	mCurrBackBufferIdx = mSwapChain->GetCurrentBackBufferIndex();

}

void DXGIMgr::CreateGraphicsRootSignature()
{
	mGraphicsRootSignature = std::make_shared<GraphicsRootSignature>();
	mGraphicsRootSignature->CreateDefaultGraphicsRootSignature();
}

void DXGIMgr::CreateComputeRootSignature()
{
	mComputeRootSignature = std::make_shared<ComputeRootSignature>();
	mComputeRootSignature->CreateDefaultComputeRootSignature();

	mParticleComputeRootSignature = std::make_shared<ComputeRootSignature>();
	mParticleComputeRootSignature->CreateParticleComputeRootSignature();
}

void DXGIMgr::CreateDescriptorHeaps(int cbvCount, int srvCount, int uavCount, int skyBoxSrvCount, int dsvCount)
{
	mDescriptorHeap->Create(cbvCount, srvCount, uavCount, skyBoxSrvCount, dsvCount);
}

void DXGIMgr::CreateDSV()
{
	mDefaultDs = res->CreateTexture("DefaultDepthStencil", mWindow.Width, mWindow.Height,
		DXGI_FORMAT_D24_UNORM_S8_UINT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	CreateDepthStencilView(mDefaultDs.get());
	CreateShaderResourceView(mDefaultDs.get(), DXGI_FORMAT_R24_UNORM_X8_TYPELESS);

	mShadowDs = res->CreateTexture("ShadowDepthStencil", mWindow.Width * 4, mWindow.Height * 4,
		DXGI_FORMAT_D24_UNORM_S8_UINT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_GENERIC_READ);
	CreateDepthStencilView(mShadowDs.get());
	CreateShaderResourceView(mShadowDs.get(), DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
}

void DXGIMgr::CreateMRTs()
{
#pragma region SwapChain
	{
		std::vector<RenderTarget> rts(mSwapChainBuffCnt);

		// 후면 버퍼 리소스를 MRT 타겟에 설정한다. 
		for (UINT i = 0; i < mSwapChainBuffCnt; ++i) {
			ComPtr<ID3D12Resource> resource;
			mSwapChain->GetBuffer(i, IID_PPV_ARGS(&resource));

			std::string name = "SwapChainTarget_" + std::to_string(i);
			rts[i].Target = res->CreateTexture(name, resource);
		}

		mMRTs[static_cast<UINT8>(GroupType::SwapChain)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::SwapChain)]->Create(GroupType::SwapChain, std::move(rts), mDefaultDs);
	}
#pragma endregion

#pragma region Shadow
	{
		// 그림자에서는 깊이 버퍼만 사용하며 어떠한 렌더 타겟도 사용하지 않는다.
		std::vector<RenderTarget> rts(0);
		mMRTs[static_cast<UINT8>(GroupType::Shadow)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::Shadow)]->Create(GroupType::Shadow, std::move(rts), mShadowDs);
	}
#pragma endregion

#pragma region GBuffer
	{
		std::vector<RenderTarget> rts(GBufferCount);

		rts[0].Target = res->CreateTexture("PositionTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[1].Target = res->CreateTexture("NormalTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[2].Target = res->CreateTexture("DiffuseTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[3].Target = res->CreateTexture("EmissiveTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[4].Target = res->CreateTexture("MetallicSmoothnessTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R8G8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[5].Target = res->CreateTexture("OcclusionTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R16_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		mMRTs[static_cast<UINT8>(GroupType::GBuffer)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::GBuffer)]->Create(GroupType::GBuffer, std::move(rts), mDefaultDs);
	}
#pragma endregion

#pragma region Lighting
	{
		std::vector<RenderTarget> rts(LightingCount);

		rts[0].Target = res->CreateTexture("DiffuseAlbedoTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[1].Target = res->CreateTexture("SpecularAlbedoTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[2].Target = res->CreateTexture("AmbientTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		mMRTs[static_cast<UINT8>(GroupType::Lighting)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::Lighting)]->Create(GroupType::Lighting, std::move(rts), mDefaultDs);
	}
#pragma endregion

#pragma region OffScreen
	{
		std::vector<RenderTarget> rts(1);

		rts[0].Target = res->CreateTexture("OffScreenTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);

		mMRTs[static_cast<UINT8>(GroupType::OffScreen)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::OffScreen)]->Create(GroupType::OffScreen, std::move(rts), mDefaultDs);
	}
#pragma endregion

#pragma region SSAO
	{
		std::vector<RenderTarget> rts(SsaoCount);

		rts[0].Target = res->CreateTexture("SSAOTarget_0", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R16_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, Vec4{ 1.f });

		rts[1].Target = res->CreateTexture("SSAOTarget_1", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R16_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, Vec4{ 1.f });

		mMRTs[static_cast<UINT8>(GroupType::Ssao)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::Ssao)]->Create(GroupType::Ssao, std::move(rts), nullptr, Vec4{ 1.f });
	}
#pragma endregion
}

void DXGIMgr::CreateFrameResources()
{
	mFrameResourceMgr = std::make_unique<FrameResourceMgr>(mFence.Get());
	mFrameResourceMgr->CreateFrameResources(mDevice.Get());
}

void DXGIMgr::ChangeSwapChainState()
{
	WaitForGpuComplete();

	mCmdList->Reset(mCmdAllocator.Get(), nullptr);

	BOOL isFullScreenState = FALSE;
	mSwapChain->GetFullscreenState(&isFullScreenState, nullptr);
	mSwapChain->SetFullscreenState(!isFullScreenState, nullptr);

	DXGI_MODE_DESC dxgiTargetParameters{};
	dxgiTargetParameters.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width                   = GetWindowWidth();
	dxgiTargetParameters.Height                  = GetWindowHeight();
	dxgiTargetParameters.RefreshRate.Numerator   = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	mSwapChain->ResizeTarget(&dxgiTargetParameters);

	// 렌더 타겟을 모두 해제한다.
	GetMRT(GroupType::SwapChain)->ReleaseRenderTargets();

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	mSwapChain->GetDesc(&swapChainDesc);
	HRESULT hResult = mSwapChain->ResizeBuffers(
		mSwapChainBuffCnt, 
		GetWindowWidth(), 
		GetWindowHeight(), 
		swapChainDesc.BufferDesc.Format, 
		swapChainDesc.Flags);
	AssertHResult(hResult);

	mCurrBackBufferIdx = mSwapChain->GetCurrentBackBufferIndex();

	// 텍스처와 MRT를 새로 생성한다.
	std::vector<RenderTarget> rts(mSwapChainBuffCnt);
	for (UINT i = 0; i < mSwapChainBuffCnt; ++i) {
		ComPtr<ID3D12Resource> resource;
		mSwapChain->GetBuffer(i, IID_PPV_ARGS(&resource));

		std::string name = "SwapChainTarget_" + std::to_string(i);
		rts[i].Target = res->CreateTexture(name, resource);
	}
	mMRTs[static_cast<UINT8>(GroupType::SwapChain)]->Create(GroupType::SwapChain, std::move(rts), mDefaultDs);

	// 윈도우 사이즈가 변경되면 필터도 다시 생성해야 한다.
	mBlurFilter->OnResize(GetWindowWidth(), GetWindowHeight());

	mCmdList->Close();
	ID3D12CommandList* cmdsLists[] = { mCmdList.Get() };
	mCmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	WaitForGpuComplete();
}

void DXGIMgr::CreateFilter()
{
	mBlurFilter = std::make_unique<BlurFilter>(GetWindowWidth(), GetWindowHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT);
	mBlurFilter->Create();

	mLUTFilter = std::make_unique<LUTFilter>(GetWindowWidth(), GetWindowHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT);
	mLUTFilter->Create();
}

void DXGIMgr::CreateSsao()
{
	mSsao = std::make_unique<Ssao>(mCmdList.Get());
}

void DXGIMgr::WaitForGpuComplete()
{
	UINT64 fenceValue = ++mFenceValues;
	mCmdQueue->Signal(mFence.Get(), fenceValue);

	if (mFence->GetCompletedValue() < fenceValue) {
		mFence->SetEventOnCompletion(fenceValue, mFenceEvent);
		::WaitForSingleObject(mFenceEvent, INFINITE);
	}
}

void DXGIMgr::MoveToNextFrame()
{
	// 다음 프레임으로 넘어갈 때 동기화를 하지 않는다.
	mCurrBackBufferIdx = mSwapChain->GetCurrentBackBufferIndex();

	mFrameResourceMgr->GetCurrFrameResource()->Fence = ++mFenceValues;

	mCmdQueue->Signal(mFence.Get(), mFenceValues);
}

void DXGIMgr::BuildScene()
{
	scene->BuildObjects();

	mCmdList->Close();
	ID3D12CommandList* cmdsLists[] = { mCmdList.Get() };
	mCmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// GPU의 모든 실행이 끝난 후 업로드 버퍼를 해제해야 한다.
	WaitForGpuComplete();

	scene->ReleaseUploadBuffers();
}