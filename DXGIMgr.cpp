#include "stdafx.h"
#include "DXGIMgr.h"
#include "Scene.h"
#include "Shader.h"

SINGLETON_PATTERN_DEFINITION(DXGIMgr)

DXGIMgr::DXGIMgr()
{
	mClientWidth = FRAME_BUFFER_WIDTH;
	mClientHeight = FRAME_BUFFER_HEIGHT;
}

bool DXGIMgr::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	mInstance = hInstance;
	mWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();

	CreateSwapChain();
	CreateRenderTargetViews();
	CreateDepthStencilView();

	BuildObjects();

	return(true);
}

void DXGIMgr::OnDestroy()
{
	::CloseHandle(mFenceEvent);
	Destroy();
}

void DXGIMgr::Terminate()
{
	BOOL bFullScreenState = FALSE;
	mSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	if (bFullScreenState) {
		ChangeSwapChainState();
	}
}


void DXGIMgr::CreateDirect3DDevice()
{
	CreateFactory();
	CreateDevice();
	SetMSAA();
	CreateFence();

	SetIncrementSize();
}

void DXGIMgr::CreateFactory()
{
	HRESULT hResult{};

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ComPtr<ID3D12Debug> debugController{};
	hResult = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	if (debugController) {
		debugController->EnableDebugLayer();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, IID_PPV_ARGS(&mFactory));
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
		else if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice)))) {
			break;
		}
	}

	if (!mDevice) {
		mFactory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		HRESULT hResult = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice));
	}
}

void DXGIMgr::SetMSAA()
{
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels{};
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 4;

	HRESULT hResult = mDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	mMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	mIsMsaa4xEnabled = (mMsaa4xQualityLevels > 1) ? true : false;
}

void DXGIMgr::CreateFence()
{
	HRESULT hResult = mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
	for (UINT i = 0; i < mSwapChainBufferCount; i++) mFenceValues[i] = 0;
	mFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}


void DXGIMgr::SetIncrementSize()
{
	::gnCbvSrvDescriptorIncrementSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}


void DXGIMgr::CreateCommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc{};
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	HRESULT hResult = mDevice->CreateCommandQueue(&d3dCommandQueueDesc, IID_PPV_ARGS(&mCmdQueue));

	hResult = mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCmdAllocator));

	hResult = mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCmdAllocator.Get(), NULL, IID_PPV_ARGS(&mCmdList));
	hResult = mCmdList->Close();
}


void DXGIMgr::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc{};
	d3dDescriptorHeapDesc.NumDescriptors = mSwapChainBufferCount + 5;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = mDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(&mRtvHeap));

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = mDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(&mDsvHeap));
}


void DXGIMgr::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(mWnd, &rcClient);
	mClientWidth = rcClient.right - rcClient.left;
	mClientHeight = rcClient.bottom - rcClient.top;

	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc{};
	dxgiSwapChainDesc.Width = mClientWidth;
	dxgiSwapChainDesc.Height = mClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (mIsMsaa4xEnabled) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (mIsMsaa4xEnabled) ? (mMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = mSwapChainBufferCount;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc{};
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = mFactory->CreateSwapChainForHwnd(mCmdQueue.Get(), mWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1**)mSwapChain.GetAddressOf());

	hResult = mFactory->MakeWindowAssociation(mWnd, DXGI_MWA_NO_ALT_ENTER);
	mSwapChainBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
}


void DXGIMgr::CreateRenderTargetViews()
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < mSwapChainBufferCount; ++i)
	{
		mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBackBuffers[i]));
		mDevice->CreateRenderTargetView(mSwapChainBackBuffers[i].Get(), &rtvDesc, rtvHandle);
		mSwapChainBackBufferRtvHandles[i] = rtvHandle;
		rtvHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}


void DXGIMgr::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = mClientWidth;
	resourceDesc.Height = mClientHeight;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = (mIsMsaa4xEnabled) ? 4 : 1;
	resourceDesc.SampleDesc.Quality = (mIsMsaa4xEnabled) ? (mMsaa4xQualityLevels - 1) : 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties{};
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue{};
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	mDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, IID_PPV_ARGS(&mDepthStencilBuffer));

	mDsvHandle = mDsvHeap->GetCPUDescriptorHandleForHeapStart();
	mDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), NULL, mDsvHandle);
}








void DXGIMgr::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL bFullScreenState = FALSE;
	mSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	mSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters{};
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = mClientWidth;
	dxgiTargetParameters.Height = mClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	mSwapChain->ResizeTarget(&dxgiTargetParameters);

	for (int i = 0; i < mSwapChainBufferCount; i++) {
		mSwapChainBackBuffers[i] = nullptr;
	}

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc{};
	mSwapChain->GetDesc(&dxgiSwapChainDesc);
	mSwapChain->ResizeBuffers(mSwapChainBufferCount, mClientWidth, mClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
	mSwapChainBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}


void DXGIMgr::WaitForGpuComplete()
{
	UINT64 nFenceValue = ++mFenceValues[mSwapChainBufferIndex];
	HRESULT hResult = mCmdQueue->Signal(mFence.Get(), nFenceValue);

	if (mFence->GetCompletedValue() < nFenceValue) {
		hResult = mFence->SetEventOnCompletion(nFenceValue, mFenceEvent);
		::WaitForSingleObject(mFenceEvent, INFINITE);
	}
}

void DXGIMgr::MoveToNextFrame()
{
	mSwapChainBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

	WaitForGpuComplete();
}


void DXGIMgr::StartCommand()
{
	mCmdList->Reset(mCmdAllocator.Get(), NULL);
}

void DXGIMgr::StopCommand()
{
	mCmdList->Close();
	ID3D12CommandList* pcmdLists[] = { mCmdList.Get() };
	mCmdQueue->ExecuteCommandLists(1, pcmdLists);

	WaitForGpuComplete();
}











void DXGIMgr::BuildObjects()
{
	StartCommand();

	Scene::Create();
	crntScene->BuildObjects();

	mPostProcessingShader = std::make_shared<TextureToFullScreenShader>();
	mPostProcessingShader->CreateShader(1, NULL, DXGI_FORMAT_D32_FLOAT);

	// 마지막으로 생성한 RTV 다음 위치에 새로운 RTV를 생성한다.
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mSwapChainBackBufferRtvHandles[mSwapChainBufferCount - 1];
	rtvHandle.ptr += ::gnRtvDescriptorIncrementSize;

	DXGI_FORMAT resourceFormats[4] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };
	mPostProcessingShader->CreateResourcesAndRtvsSrvs(4, resourceFormats, rtvHandle); //SRV to (Render Targets) + (Depth Buffer)

	// create SRV for DepthStencil buffer
	crntScene->CreateShaderResourceView(mDepthStencilBuffer, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);

	StopCommand();

	crntScene->ReleaseUploadBuffers();
}



void DXGIMgr::Render()
{
	HRESULT hResult = mCmdAllocator->Reset();
	StartCommand();

	// set before barrier (present -> render target)
	::SynchronizeResourceTransition(mSwapChainBackBuffers[mSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	if (mDrawOption == DRAW_SCENE_COLOR) {
		// clear DSV
		mCmdList->ClearDepthStencilView(mDsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

		// OMSetRenderTarget
		mPostProcessingShader->OnPrepareRenderTarget(&mSwapChainBackBufferRtvHandles[mSwapChainBufferIndex], &mDsvHandle);

		// render scene
		crntScene->Render();

		// for SynchronizeResourceTransition
		mPostProcessingShader->OnPostRenderTarget();

		// post processing
		cmdList->OMSetRenderTargets(1, &mSwapChainBackBufferRtvHandles[mSwapChainBufferIndex], TRUE, NULL);
		mPostProcessingShader->Render();
	}
	else {
		crntScene->OnPrepareRender();

		mCmdList->OMSetRenderTargets(1, &mSwapChainBackBufferRtvHandles[mSwapChainBufferIndex], TRUE, NULL);

		mPostProcessingShader->Render();
	}

	// set after barrier (render target -> present)
	::SynchronizeResourceTransition(mSwapChainBackBuffers[mSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	StopCommand();

	mSwapChain->Present(0, 0);

	MoveToNextFrame();
}





void DXGIMgr::ToggleFullScreen()
{
	ChangeSwapChainState();
}

void DXGIMgr::SetDrawOption(int option)
{
	mDrawOption = option;
}

void DXGIMgr::ClearStencil()
{
	mCmdList->ClearDepthStencilView(mDsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
}
