#include "stdafx.h"
#include "DXGIMgr.h"
#include "FrameResource.h"

#include "Scene.h"
#include "Shader.h"
#include "Texture.h"
#include "MultipleRenderTarget.h"
#include "BlurFilter.h"
#include "LUTFilter.h"

#pragma region Imgui - 장재문 - 일단 여기다가 넣겠습니다.
#include "../Imgui/ImguiMgr.h"
#pragma endregion


DXGIMgr::DXGIMgr()
	:
	mClientWidth(gkFrameBufferWidth),
	mClientHeight(gkFrameBufferHeight)
{
	mFilterOption = FilterOption::LUT;
}

void DXGIMgr::Init(HINSTANCE hInstance, HWND hMainWnd)
{
	mInstance = hInstance;
	mWnd      = hMainWnd;

	CreateDirect3DDevice();
	CreateCmdQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();

	CreateSwapChain();
	CreateDSV();

	CreateFrameResources();

	BuildScene();

	CreateMRTs();
	CreateFilter();
}

void DXGIMgr::Release()
{
	::CloseHandle(mFenceEvent);
	Destroy();
}

void DXGIMgr::Terminate()
{
	BOOL isFullScreenState = FALSE;
	mSwapChain->GetFullscreenState(&isFullScreenState, NULL);

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

	RenderBegin();

#pragma region ClearRTVs
	// 해당 함수들 안에서 자신이 사용할 깊이 버퍼를 클리어 한다.
	GetMRT(GroupType::SwapChain)->ClearRenderTargetView(mCurrBackBufferIdx);
	GetMRT(GroupType::GBuffer)->ClearRenderTargetView();
	GetMRT(GroupType::OffScreen)->ClearRenderTargetView();
#pragma endregion

#pragma region MainRender
	switch (mDrawOption) {
	case DrawOption::Main:
	{
		// 그림자 맵 텍스처를 렌더 타겟으로 설정하고 그림자 렌더링
		scene->RenderShadow();

		// GBuffer를 렌더 타겟으로 설정하고 디퍼드 렌더링
		GetMRT(GroupType::GBuffer)->OMSetRenderTargets();
		scene->RenderDeferred();
		GetMRT(GroupType::GBuffer)->WaitTargetToResource();

		// 라이트 맵 텍스처를 렌더 타겟으로 설정하고 라이트 렌더링
		scene->RenderLights();

		// 후면 버퍼대신 화면 밖 텍스처를 렌더 타겟으로 설정하고 렌더링
		GetMRT(GroupType::OffScreen)->OMSetRenderTargets();
		scene->RenderFinal();
		scene->RenderForward();
	}
	break;
	}
#pragma endregion

#pragma region PostProcessing
	UINT offScreenIndex = GetMRT(GroupType::OffScreen)->GetTexture(OffScreen::Texture)->GetGpuDescriptorHandleIndex();
	switch (mFilterOption)
	{
	case FilterOption::Blur:
		offScreenIndex = mBlurFilter->Execute(GetMRT(GroupType::OffScreen)->GetTexture(OffScreen::Texture), 4);
		break;
	case FilterOption::LUT:
		offScreenIndex = mLUTFilter->Execute(GetMRT(GroupType::OffScreen)->GetTexture(OffScreen::Texture));
		break;
	}

	GetMRT(GroupType::SwapChain)->OMSetRenderTargets(1, mCurrBackBufferIdx);
	scene->RenderPostProcessing(offScreenIndex);
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

void DXGIMgr::ClearDepth()
{
	mCmdList->ClearDepthStencilView(mDsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, NULL);
}
void DXGIMgr::ClearStencil()
{
	mCmdList->ClearDepthStencilView(mDsvHandle, D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, NULL);
}
void DXGIMgr::ClearDepthStencil()
{
	mCmdList->ClearDepthStencilView(mDsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, NULL);
}

void DXGIMgr::RenderBegin()
{
	auto& cmdAllocator = mFrameResourceMgr->GetCurrFrameResource()->CmdAllocator;

	mCmdList->Reset(cmdAllocator.Get(), NULL);
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
	hResult = mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCmdAllocator.Get(), NULL, IID_PPV_ARGS(&mCmdList));
	AssertHResult(hResult);
	hResult = mCmdList->Close();
	AssertHResult(hResult);
}


void DXGIMgr::CreateRtvAndDsvDescriptorHeaps()
{
	// Create DSV
	constexpr int kDepthStencilBuffCnt = 1; // Depth 버퍼는 1개만 사용한다.

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.NumDescriptors = kDepthStencilBuffCnt;
	descriptorHeapDesc.Type			  = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	descriptorHeapDesc.Flags		  = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descriptorHeapDesc.NodeMask		  = 0;

	HRESULT hResult = mDevice->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&mDsvHeap));
	AssertHResult(hResult);
}


void DXGIMgr::CreateSwapChain()
{
	// client의 screen rect를 OS로부터 받아와 설정한다.
	RECT clientRect;
	::GetClientRect(mWnd, &clientRect);
	mClientWidth  = clientRect.right - clientRect.left;
	mClientHeight = clientRect.bottom - clientRect.top;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width              = mClientWidth;
	swapChainDesc.Height             = mClientHeight;
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

	HRESULT hResult   = mFactory->CreateSwapChainForHwnd(mCmdQueue.Get(), mWnd, &swapChainDesc, &swapChainFullScreenDesc, NULL, (IDXGISwapChain1**)mSwapChain.GetAddressOf());
	AssertHResult(hResult);

	hResult           = mFactory->MakeWindowAssociation(mWnd, DXGI_MWA_NO_ALT_ENTER);
	AssertHResult(hResult);

	mCurrBackBufferIdx = mSwapChain->GetCurrentBackBufferIndex();

}

void DXGIMgr::CreateDSV()
{
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment          = 0;
	resourceDesc.Width              = mClientWidth;
	resourceDesc.Height             = mClientHeight;
	resourceDesc.DepthOrArraySize   = 1;
	resourceDesc.MipLevels          = 1;
	resourceDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;	// Depth buffer 24bit, stencil buffer 8bit
	resourceDesc.SampleDesc.Count   = (mIsMsaa4xEnabled) ? 4 : 1;
	resourceDesc.SampleDesc.Quality = (mIsMsaa4xEnabled) ? (mMsaa4xQualityLevels - 1) : 0;
	resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type                 = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask     = 1;
	heapProperties.VisibleNodeMask      = 1;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format               = DXGI_FORMAT_D24_UNORM_S8_UINT;
	clearValue.DepthStencil.Depth   = 1.f;
	clearValue.DepthStencil.Stencil = 0;

	HRESULT hResult = mDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&mDepthStencilBuff));
	AssertHResult(hResult);

	mDsvHandle = mDsvHeap->GetCPUDescriptorHandleForHeapStart();
	mDevice->CreateDepthStencilView(mDepthStencilBuff.Get(), NULL, mDsvHandle);
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
			rts[i].Target = std::make_shared<Texture>(D3DResource::Texture2D);
			rts[i].Target->CreateTextureFromResource(resource);
		}

		mMRTs[static_cast<UINT8>(GroupType::SwapChain)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::SwapChain)]->Create(GroupType::SwapChain, std::move(rts), mDsvHandle);
	}
#pragma endregion

#pragma region GBuffer
	{
		std::vector<RenderTarget> rts(GBufferCount);

		rts[0].Target = std::make_shared<Texture>(D3DResource::Texture2D);
		rts[0].Target->CreateTexture(gkFrameBufferWidth, gkFrameBufferHeight,
			DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[1].Target = std::make_shared<Texture>(D3DResource::Texture2D);
		rts[1].Target->CreateTexture(gkFrameBufferWidth, gkFrameBufferHeight,
			DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[2].Target = std::make_shared<Texture>(D3DResource::Texture2D);
		rts[2].Target->CreateTexture(gkFrameBufferWidth, gkFrameBufferHeight,
			DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[3].Target = std::make_shared<Texture>(D3DResource::Texture2D);
		rts[3].Target->CreateTexture(gkFrameBufferWidth, gkFrameBufferHeight,
			DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[4].Target = std::make_shared<Texture>(D3DResource::Texture2D);
		rts[4].Target->CreateTexture(gkFrameBufferWidth, gkFrameBufferHeight,
			DXGI_FORMAT_R32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		mMRTs[static_cast<UINT8>(GroupType::GBuffer)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::GBuffer)]->Create(GroupType::GBuffer, std::move(rts), mDsvHandle);
	}
#pragma endregion

#pragma region OffScreen
	{
		std::vector<RenderTarget> rts(OffScreenCount);

		rts[0].Target = std::make_shared<Texture>(D3DResource::Texture2D);
		rts[0].Target->CreateTexture(gkFrameBufferWidth, gkFrameBufferHeight,
			DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);

		mMRTs[static_cast<UINT8>(GroupType::OffScreen)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::OffScreen)]->Create(GroupType::OffScreen, std::move(rts), mDsvHandle);
	}
#pragma endregion

	// create SRV for DepthStencil buffer
	scene->CreateShaderResourceView(mDepthStencilBuff, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
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
	mSwapChain->GetFullscreenState(&isFullScreenState, NULL);
	mSwapChain->SetFullscreenState(!isFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters{};
	dxgiTargetParameters.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width                   = mClientWidth;
	dxgiTargetParameters.Height                  = mClientHeight;
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
		mClientWidth, 
		mClientHeight, 
		swapChainDesc.BufferDesc.Format, 
		swapChainDesc.Flags);
	AssertHResult(hResult);

	mCurrBackBufferIdx = mSwapChain->GetCurrentBackBufferIndex();

	// 텍스처와 MRT를 새로 생성한다.
	std::vector<RenderTarget> rts(mSwapChainBuffCnt);
	for (UINT i = 0; i < mSwapChainBuffCnt; ++i) {
		ComPtr<ID3D12Resource> resource;
		mSwapChain->GetBuffer(i, IID_PPV_ARGS(&resource));
		rts[i].Target = std::make_shared<Texture>(D3DResource::Texture2D);
		rts[i].Target->CreateTextureFromResource(resource);
	}
	mMRTs[static_cast<UINT8>(GroupType::SwapChain)]->Create(GroupType::SwapChain, std::move(rts), mDsvHandle);

	// 윈도우 사이즈가 변경되면 필터도 다시 생성해야 한다.
	mBlurFilter->OnResize(mClientWidth, mClientHeight);

	mCmdList->Close();
	ID3D12CommandList* cmdsLists[] = { mCmdList.Get() };
	mCmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	WaitForGpuComplete();
}

void DXGIMgr::CreateFilter()
{
	mBlurFilter = std::make_unique<BlurFilter>(mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
	mBlurFilter->Create();

	mLUTFilter = std::make_unique<LUTFilter>(mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
	mLUTFilter->Create();
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
	// 처음 빌드할때는 프레임 리소스의 명령 할당자가 아니어야 한다.
	mCmdList->Reset(mCmdAllocator.Get(), nullptr);

	scene->BuildObjects();

	mCmdList->Close();
	ID3D12CommandList* cmdsLists[] = { mCmdList.Get() };
	mCmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// GPU의 모든 실행이 끝난 후 업로드 버퍼를 해제해야 한다.
	WaitForGpuComplete();

	scene->ReleaseUploadBuffers();
}