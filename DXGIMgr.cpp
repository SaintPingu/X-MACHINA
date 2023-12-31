#include "stdafx.h"
#include "DXGIMgr.h"
#include "Scene.h"
#include "Shader.h"

SINGLETON_PATTERN_DEFINITION(DXGIMgr)

DXGIMgr::DXGIMgr()
{
	mClientWidth  = gkFrameBufferWidth;
	mClientHeight = gkFrameBufferHeight;
}

void DXGIMgr::Init(HINSTANCE hInstance, HWND hMainWnd)
{
	mInstance = hInstance;
	mWnd      = hMainWnd;

	CreateDirect3DDevice();
	CreateCmdQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();

	CreateSwapChain();
	CreateRTVs();
	CreateDSV();

	BuildObjects();

	mDrawOption = DrawOption::Main;
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


void DXGIMgr::Render()
{
	mCmdAllocator->Reset();
	StartCommand();

	// set before barrier (present -> render target)
	D3DUtil::ResourceTransition(mSwapChainBuffers[mSwapChainBuffIdx].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	switch (mDrawOption) {
	case DrawOption::Main:
	{
		// clear DSV
		ClearDepthStencil();

		// OMSetRenderTarget
		mPostProcessingShader->OnPrepareRenderTarget(&mSwapChainBuffRtvHandles[mSwapChainBuffIdx], &mDsvHandle);

		// render scene
		scene->Render();

		// for ResourceTransition
		mPostProcessingShader->OnPostRenderTarget();

		// post processing
		mCmdList->OMSetRenderTargets(1, &mSwapChainBuffRtvHandles[mSwapChainBuffIdx], TRUE, NULL);
		mPostProcessingShader->Render();
	}

	break;
	default:
	{
		scene->OnPrepareRender();

		mCmdList->OMSetRenderTargets(1, &mSwapChainBuffRtvHandles[mSwapChainBuffIdx], TRUE, NULL);

		mPostProcessingShader->Render();
	}
	break;
	}

	// set after barrier (render target -> present)
	D3DUtil::ResourceTransition(mSwapChainBuffers[mSwapChainBuffIdx].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	StopCommand();

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



void DXGIMgr::StartCommand()
{
	mCmdList->Reset(mCmdAllocator.Get(), NULL);
}

void DXGIMgr::StopCommand()
{
	mCmdList->Close();
	ID3D12CommandList* cmdLists[] = { mCmdList.Get() };
	mCmdQueue->ExecuteCommandLists(1, cmdLists);

	WaitForGpuComplete();
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
	for (UINT i = 0; i < mSwapChainBuffCnt; i++) {
		mFenceValues[i] = 0;
	}
	mFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}


void DXGIMgr::SetIncrementSize()
{
	mCbvSrvDescriptorIncSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	mRtvDescriptorIncSize    = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
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
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc{};
	d3dDescriptorHeapDesc.NumDescriptors = mSwapChainBuffCnt + 5;
	d3dDescriptorHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask       = 0;
	HRESULT hResult                      = mDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(&mRtvHeap));
	AssertHResult(hResult);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult                              = mDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(&mDsvHeap));
	AssertHResult(hResult);
}


void DXGIMgr::CreateSwapChain()
{
	RECT clientRect;
	::GetClientRect(mWnd, &clientRect);
	mClientWidth  = clientRect.right - clientRect.left;
	mClientHeight = clientRect.bottom - clientRect.top;

	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc{};
	dxgiSwapChainDesc.Width              = mClientWidth;
	dxgiSwapChainDesc.Height             = mClientHeight;
	dxgiSwapChainDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count   = (mIsMsaa4xEnabled) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (mIsMsaa4xEnabled) ? (mMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount        = mSwapChainBuffCnt;
	dxgiSwapChainDesc.Scaling            = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc{};
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator   = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed                = TRUE;

	HRESULT hResult   = mFactory->CreateSwapChainForHwnd(mCmdQueue.Get(), mWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1**)mSwapChain.GetAddressOf());
	AssertHResult(hResult);

	hResult           = mFactory->MakeWindowAssociation(mWnd, DXGI_MWA_NO_ALT_ENTER);
	AssertHResult(hResult);

	mSwapChainBuffIdx = mSwapChain->GetCurrentBackBufferIndex();

}


void DXGIMgr::CreateRTVs()
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format               = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice   = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < mSwapChainBuffCnt; ++i) {
		mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffers[i]));
		mDevice->CreateRenderTargetView(mSwapChainBuffers[i].Get(), &rtvDesc, rtvHandle);
		mSwapChainBuffRtvHandles[i] = rtvHandle;
		rtvHandle.ptr += mRtvDescriptorIncSize;
	}
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
	resourceDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
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


void DXGIMgr::ChangeSwapChainState()
{
	WaitForGpuComplete();

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

	for (int i = 0; i < mSwapChainBuffCnt; i++) {
		mSwapChainBuffers[i] = nullptr;
	}

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc{};
	mSwapChain->GetDesc(&dxgiSwapChainDesc);
	HRESULT hResult = mSwapChain->ResizeBuffers(mSwapChainBuffCnt, mClientWidth, mClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
	AssertHResult(hResult);

	mSwapChainBuffIdx = mSwapChain->GetCurrentBackBufferIndex();

	CreateRTVs();
}

void DXGIMgr::WaitForGpuComplete()
{
	UINT64 fenceValue = ++mFenceValues[mSwapChainBuffIdx];
	mCmdQueue->Signal(mFence.Get(), fenceValue);

	if (mFence->GetCompletedValue() < fenceValue) {
		mFence->SetEventOnCompletion(fenceValue, mFenceEvent);
		::WaitForSingleObject(mFenceEvent, INFINITE);
	}
}

void DXGIMgr::MoveToNextFrame()
{
	mSwapChainBuffIdx = mSwapChain->GetCurrentBackBufferIndex();

	WaitForGpuComplete();
}

void DXGIMgr::BuildObjects()
{
	StartCommand();

	Scene::Create();
	scene->BuildObjects();

	mPostProcessingShader = std::make_shared<TextureToScreenShader>();
	mPostProcessingShader->Create(DXGI_FORMAT_D32_FLOAT);

	// 마지막으로 생성한 RTV 다음 위치에 새로운 RTV를 생성한다.
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mSwapChainBuffRtvHandles[mSwapChainBuffCnt - 1];
	rtvHandle.ptr                        += mRtvDescriptorIncSize;

	mPostProcessingShader->CreateResourcesAndRtvsSrvs(rtvHandle); //SRV to (Render Targets) + (Depth Buffer)

	// create SRV for DepthStencil buffer
	scene->CreateShaderResourceView(mDepthStencilBuff, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);

	StopCommand();

	scene->ReleaseUploadBuffers();
}