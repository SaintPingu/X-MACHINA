#include "EnginePch.h"
#include "DXGIMgr.h"
#include "FrameResource.h"
#include "DescriptorHeap.h"

#include "ResourceMgr.h"
#include "BattleScene.h"
#include "LobbyScene.h"
#include "LoadingScene.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "RootSignature.h"
#include "MultipleRenderTarget.h"
#include "BlurFilter.h"
#include "LUTFilter.h"
#include "SobelFilter.h"
#include "Ssao.h"
#include "Bloom.h"
#include "MeshRenderer.h"

#include "InputMgr.h"
#include "TextMgr.h"
#include "Component/UI.h"
#include "Component/ParticleSystem.h"
#include "Object.h"

#pragma region Imgui
#include "../Imgui/ImGuiMgr.h"
#pragma endregion


DXGIMgr::DXGIMgr()
	:
	mDescriptorHeap(std::make_shared<DescriptorHeap>())
{
	DWORD filterOption = FilterOption::None;

#ifndef RENDER_FOR_SERVER

	//filterOption |= FilterOption::Blur;
	filterOption |= FilterOption::LUT;
	filterOption |= FilterOption::Tone;
	filterOption |= FilterOption::Ssao;
	filterOption |= FilterOption::Shadow;
	filterOption |= FilterOption::Bloom;
	filterOption |= FilterOption::Sobel;

#endif

	mFilterOption = filterOption;
}

DXGIMgr::~DXGIMgr()
{
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
	CMD_LIST->SetGraphicsRoot32BitConstants(GetGraphicsRootParamIndex(param), kNum32Bit, &data, offset);
}

void DXGIMgr::SetGraphicsRoot32BitConstants(RootParam param, const Vec4& data, UINT offset)
{
	constexpr UINT kNum32Bit = 4U;
	CMD_LIST->SetGraphicsRoot32BitConstants(GetGraphicsRootParamIndex(param), kNum32Bit, &data, offset);
}

void DXGIMgr::SetGraphicsRoot32BitConstants(RootParam param, float data, UINT offset)
{
	constexpr UINT kNum32Bit = 1U;
	CMD_LIST->SetGraphicsRoot32BitConstants(GetGraphicsRootParamIndex(param), kNum32Bit, &data, offset);
}

void DXGIMgr::SetGraphicsRoot32BitConstants(RootParam param, int data, UINT offset)
{
	constexpr UINT kNum32Bit = 1U;
	CMD_LIST->SetGraphicsRoot32BitConstants(GetGraphicsRootParamIndex(param), kNum32Bit, &data, offset);
}

void DXGIMgr::SetGraphicsRootConstantBufferView(RootParam param, D3D12_GPU_VIRTUAL_ADDRESS gpuAddr)
{
	CMD_LIST->SetGraphicsRootConstantBufferView(GetGraphicsRootParamIndex(param), gpuAddr);
}

void DXGIMgr::SetGraphicsRootShaderResourceView(RootParam param, D3D12_GPU_VIRTUAL_ADDRESS gpuAddr)
{
	CMD_LIST->SetGraphicsRootShaderResourceView(GetGraphicsRootParamIndex(param), gpuAddr);
}

void DXGIMgr::Init(HINSTANCE hInstance, const WindowInfo& window)
{
	mCrntScene = LobbyScene::I.get();

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
	CreateDirect2DDevice();
	CreateFilter();
	CreateSsao();

	ResourceMgr::I->LoadResources();
	ParticleManager::I->Init();
	DynamicEnvironmentMappingManager::I->Init();
	Canvas::I->Init();

	BuildScene();

	InputMgr::I->UpdateClient();

	//ChangeSwapChainState();
}

void DXGIMgr::Release()
{
	Terminate();

	FRAME_RESOURCE_MGR->WaitForGpuComplete();

	mGraphicsRootSignature = nullptr;
	mComputeRootSignature = nullptr;
	::CloseHandle(mFenceEvent);
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

	// 커맨드 리스트, 할당자 오픈
	auto& cmdAllocator = mFrameResourceMgr->GetCurrFrameResource()->CmdAllocator;
	cmdAllocator->Reset();
	mCmdList->Reset(cmdAllocator.Get(), NULL);

	MainPassRenderBegin();
}

void DXGIMgr::ToggleFullScreen()
{
	ChangeSwapChainState();
}

void DXGIMgr::SwitchScene(SceneType sceneType)
{
	switch (sceneType) {
	case SceneType::Lobby:
		mCrntScene = LobbyScene::I.get();
		break;
	case SceneType::Battle:
		mCrntScene = BattleScene::I.get();
		break;
	case SceneType::Loading:
		mCrntScene = LoadingScene::I.get();
		break;
	default:
		assert(0);
		break;
	}
}

void DXGIMgr::RenderScene()
{
	GetMRT(GroupType::SwapChain)->ClearRenderTargetView(mCurrBackBufferIdx, 1.f);
	GetMRT(GroupType::GBuffer)->ClearRenderTargetView(1.f);
	GetMRT(GroupType::Lighting)->ClearRenderTargetView(1.f);
	GetMRT(GroupType::OffScreen)->ClearRenderTargetView(0, 1.f);
	GetMRT(GroupType::Shadow)->ClearRenderTargetView(1.f);

#ifdef RENDER_FOR_SERVER

	RenderDeferred();
	RenderLights();
	RenderOffScreen();
	RenderPostProcessing();
	RenderUI();

#else

	RenderShadow();
	RenderEnvironmentMapping();
	RenderDeferred();
	RenderSSAO();
	RenderLights();
	RenderCustomDepth();
	RenderOffScreen();
	RenderBloom();
	RenderPostProcessing();
	RenderUI();

#endif

	RenderEnd();

	//RenderText();

	mSwapChain->Present(1, 0);

	MoveToNextFrame();

	mCrntScene->RenderEnd();
}

void DXGIMgr::RenderShadow()
{
	if (!GetFilterOption(FilterOption::Shadow))
		return;

	CMD_LIST->SetGraphicsRootConstantBufferView(GetGraphicsRootParamIndex(RootParam::Pass), FRAME_RESOURCE_MGR->GetPassCBGpuAddr(1));

	GetMRT(GroupType::Shadow)->OMSetRenderTargets(0, 0);
	mCrntScene->RenderShadow();
	GetMRT(GroupType::Shadow)->WaitTargetToResource();
}

void DXGIMgr::RenderEnvironmentMapping()
{
	mCrntScene->ApplyDynamicContext();
}

void DXGIMgr::RenderDeferred()
{
	CMD_LIST->SetGraphicsRootConstantBufferView(GetGraphicsRootParamIndex(RootParam::Pass), FRAME_RESOURCE_MGR->GetPassCBGpuAddr(0));

	GetMRT(GroupType::GBuffer)->OMSetRenderTargets();
	mCrntScene->RenderDeferred();
	GetMRT(GroupType::GBuffer)->WaitTargetToResource();
}

void DXGIMgr::RenderSSAO()
{
	if (mFilterOption & FilterOption::Ssao) {
		mSsao->Execute(1);
	}
}

void DXGIMgr::RenderLights()
{
	GetMRT(GroupType::Lighting)->OMSetRenderTargets();
	mCrntScene->RenderLights();
	GetMRT(GroupType::Lighting)->WaitTargetToResource();
}

void DXGIMgr::RenderCustomDepth()
{
	if (!GetFilterOption(FilterOption::Custom))
		return;

	GetMRT(GroupType::CustomDepth)->ClearRenderTargetView(0.f);
	GetMRT(GroupType::CustomDepth)->OMSetRenderTargets(0, 0);
	mCrntScene->RenderCustomDepth();
	GetMRT(GroupType::CustomDepth)->WaitTargetToResource();
}

void DXGIMgr::RenderOffScreen()
{
	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	GetMRT(GroupType::GBuffer)->WaitResourceToTarget(static_cast<UINT8>(GBuffer::Normal));
	GetMRT(GroupType::GBuffer)->WaitResourceToTarget(static_cast<UINT8>(GBuffer::Emissive));
	GetMRT(GroupType::OffScreen)->OMSetRenderTargets();
	RESOURCE<Shader>("Final")->Set();
	RESOURCE<ModelObjectMesh>("Rect")->Render();
	mCrntScene->RenderForward();
	GetMRT(GroupType::OffScreen)->WaitTargetToResource();
}

void DXGIMgr::RenderBloom()
{
	if (mFilterOption & FilterOption::Bloom)
	{
		CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		mBloom->Execute();
		GetMRT(GroupType::OffScreen)->WaitResourceToTarget(0);
		GetMRT(GroupType::OffScreen)->OMSetRenderTargets(1, 0);
		RESOURCE<Shader>("Bloom")->Set();
		RESOURCE<ModelObjectMesh>("Rect")->Render();
		GetMRT(GroupType::OffScreen)->WaitTargetToResource(0);
	}
}

void DXGIMgr::RenderPostProcessing()
{
	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	PostPassRenderBegin();

	UINT offScreenIndex{};
	UINT outlineIndex{};

#ifdef RENDER_FOR_SERVER

	offScreenIndex = GetMRT(GroupType::OffScreen)->GetTexture(0)->GetSrvIdx();

#else

	if (mFilterOption & FilterOption::None)
		offScreenIndex = GetMRT(GroupType::OffScreen)->GetTexture(0)->GetSrvIdx();
	if (mFilterOption & FilterOption::Sobel)
		outlineIndex = mSobelFilter->Execute(GetMRT(GroupType::GBuffer)->GetTexture(GBuffer::Outline));
	if (mFilterOption & FilterOption::Blur)
		offScreenIndex = mBlurFilter->Execute(GetMRT(GroupType::OffScreen)->GetTexture(0), 4);
	if (mFilterOption & FilterOption::LUT || mFilterOption & FilterOption::Tone)
		offScreenIndex = mLUTFilter->Execute(GetMRT(GroupType::OffScreen)->GetTexture(0));
#endif

	GetMRT(GroupType::SwapChain)->OMSetRenderTargets(1, mCurrBackBufferIdx);

	PostPassConstants passConstants;
	passConstants.RT0_OffScreenIndex = offScreenIndex;
	passConstants.OutlineIndex = outlineIndex;
	FRAME_RESOURCE_MGR->CopyData(passConstants);
	CMD_LIST->SetGraphicsRootConstantBufferView(GetGraphicsRootParamIndex(RootParam::PostPass), FRAME_RESOURCE_MGR->GetPostPassCBGpuAddr());

	RESOURCE<Shader>("OffScreen")->Set();
	RESOURCE<ModelObjectMesh>("Rect")->Render();
}

void DXGIMgr::RenderUI()
{
	mCrntScene->RenderUI();
}

void DXGIMgr::MainPassRenderBegin()
{
	mCrntScene->RenderBegin();

	mDescriptorHeap->Set();

	// 그래픽스 쉐이더 리소스 연결
	CMD_LIST->SetGraphicsRootSignature(GetGraphicsRootSignature().Get());
	CMD_LIST->SetGraphicsRootConstantBufferView(GetGraphicsRootParamIndex(RootParam::Ssao), FRAME_RESOURCE_MGR->GetSSAOCBGpuAddr());
	CMD_LIST->SetGraphicsRootShaderResourceView(GetGraphicsRootParamIndex(RootParam::Material), FRAME_RESOURCE_MGR->GetMatBufferGpuAddr());
	CMD_LIST->SetGraphicsRootDescriptorTable(GetGraphicsRootParamIndex(RootParam::Texture), mDescriptorHeap->GetGPUHandle());
	CMD_LIST->SetGraphicsRootDescriptorTable(GetGraphicsRootParamIndex(RootParam::SkyBox), mDescriptorHeap->GetSkyBoxGPUStartSrvHandle());

	// 컴퓨트 쉐이더 리소스 연결
	CMD_LIST->SetComputeRootSignature(GetParticleComputeRootSignature().Get());
	CMD_LIST->SetComputeRootShaderResourceView(GetParticleComputeRootParamIndex(RootParam::ParticleSystem), FRAME_RESOURCE_MGR->GetParticleSystemGpuAddr());
	CMD_LIST->SetComputeRootUnorderedAccessView(GetParticleComputeRootParamIndex(RootParam::ParticleShared), FRAME_RESOURCE_MGR->GetParticleSharedGpuAddr());
}

void DXGIMgr::PostPassRenderBegin()
{
	CMD_LIST->SetComputeRootSignature(GetComputeRootSignature().Get());
}

void DXGIMgr::RenderText()
{
	mD3D11On12Device->AcquireWrappedResources(mWrappedBackBuffers[mCurrBackBufferIdx].GetAddressOf(), 1);

	mD2DDeviceContext->SetTarget(mBitmapRenderTargets[mCurrBackBufferIdx].Get());
	mD2DDeviceContext->BeginDraw();

	mCrntScene->RenderText(mD2DDeviceContext);

	THROW_IF_FAILED(mD2DDeviceContext->EndDraw());

	mD3D11On12Device->ReleaseWrappedResources(mWrappedBackBuffers[mCurrBackBufferIdx].GetAddressOf(), 1);

	mD3D11DeviceContext->Flush();
}

void DXGIMgr::RenderEnd()
{
	mCmdList->Close();
	ID3D12CommandList* cmdLists[] = { mCmdList.Get() };
	mCmdQueue->ExecuteCommandLists(1, cmdLists);
}

void DXGIMgr::CreateFactory()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	ComPtr<ID3D12Debug> debugController{};
	THROW_IF_FAILED(::D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	if (debugController) {
		debugController->EnableDebugLayer();
	}
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	THROW_IF_FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mFactory)));
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
		THROW_IF_FAILED(::D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice)));
	}
}

void DXGIMgr::CreateD2DDevice()
{
	D2D1_FACTORY_OPTIONS d2dFactoryOptions{};
	D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
	THROW_IF_FAILED(::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &mD2DFactory));

	ComPtr<IDXGIDevice> dxgiDevice;
	THROW_IF_FAILED(mD3D11On12Device.As(&dxgiDevice));
	THROW_IF_FAILED(mD2DFactory->CreateDevice(dxgiDevice.Get(), &mD2DDevice));
	THROW_IF_FAILED(mD2DDevice->CreateDeviceContext(deviceOptions, &mD2DDeviceContext));
	THROW_IF_FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &mDWriteFactory));
}

void DXGIMgr::CreateD3D11On12Device()
{
	ComPtr<ID3D11Device> d3d11Device;

	THROW_IF_FAILED(::D3D11On12CreateDevice(
		mDevice.Get(),
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		nullptr,
		0,
		reinterpret_cast<IUnknown**>(mCmdQueue.GetAddressOf()),
		1,
		0,
		&d3d11Device,
		&mD3D11DeviceContext,
		nullptr
	));
	THROW_IF_FAILED(d3d11Device.As(&mD3D11On12Device));
}

void DXGIMgr::SetMSAA()
{
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels{};
	msaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msaaQualityLevels.SampleCount = 4;
	msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msaaQualityLevels.NumQualityLevels = 4;

	THROW_IF_FAILED(mDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS)));
	mMsaa4xQualityLevels = msaaQualityLevels.NumQualityLevels;
	mIsMsaa4xEnabled = (mMsaa4xQualityLevels > 1) ? true : false;
}

void DXGIMgr::CreateFence()
{
	THROW_IF_FAILED(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
	mFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

void DXGIMgr::SetIncrementSize()
{
	mCbvSrvUavDescriptorIncSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	mRtvDescriptorIncSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvDescriptorIncSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void DXGIMgr::CreateDirect3DDevice()
{
	CreateFactory();
	CreateDevice();

	SetMSAA();
	CreateFence();

	SetIncrementSize();
}

void DXGIMgr::CreateDirect2DDevice()
{
	CreateD3D11On12Device();
	CreateD2DDevice();
	CreateD2DRTV();
}

void DXGIMgr::CreateCmdQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc{};
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	THROW_IF_FAILED(mDevice->CreateCommandQueue(&d3dCommandQueueDesc, IID_PPV_ARGS(&mCmdQueue)));
	THROW_IF_FAILED(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCmdAllocator)));
	THROW_IF_FAILED(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCmdAllocator.Get(), nullptr, IID_PPV_ARGS(&mCmdList)));
}

void DXGIMgr::CreateSwapChain()
{
	RECT clientRect;
	::GetClientRect(mWindow.Hwnd, &clientRect);
	mWindow.Width = static_cast<short>(clientRect.right - clientRect.left);
	mWindow.Height = static_cast<short>(clientRect.bottom - clientRect.top);

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = mWindow.Width;
	swapChainDesc.Height = mWindow.Height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = (mIsMsaa4xEnabled) ? 4 : 1;
	swapChainDesc.SampleDesc.Quality = (mIsMsaa4xEnabled) ? (mMsaa4xQualityLevels - 1) : 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = mSwapChainBuffCnt;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullScreenDesc{};
	swapChainFullScreenDesc.RefreshRate.Numerator = 60;
	swapChainFullScreenDesc.RefreshRate.Denominator = 1;
	swapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainFullScreenDesc.Windowed = TRUE;

	THROW_IF_FAILED(mFactory->CreateSwapChainForHwnd(mCmdQueue.Get(), GetHwnd(), &swapChainDesc, &swapChainFullScreenDesc, nullptr, (IDXGISwapChain1**)mSwapChain.GetAddressOf()));
	THROW_IF_FAILED(mFactory->MakeWindowAssociation(GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

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

void DXGIMgr::CreateD2DRTV()
{
	for (UINT i = 0; i < mSwapChainBuffCnt; ++i) {
		mWrappedBackBuffers[i].Reset();
		mBitmapRenderTargets[i].Reset();
	}

	float dpiX;
	float dpiY;

#pragma warning(push)
#pragma warning(disable : 4996)
	mD2DFactory->GetDesktopDpi(&dpiX, &dpiY);
#pragma warning(pop)

	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		dpiX,
		dpiY
	);

	D3D11_RESOURCE_FLAGS flags{ D3D11_BIND_RENDER_TARGET };
	for (UINT i = 0; i < mSwapChainBuffCnt; ++i) {
		mD3D11On12Device->CreateWrappedResource(
			mMRTs[static_cast<UINT8>(GroupType::SwapChain)]->GetTexture(i)->GetResource().Get(),
			&flags,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT,
			IID_PPV_ARGS(&mWrappedBackBuffers[i])
		);

		ComPtr<IDXGISurface> surface;
		THROW_IF_FAILED(mWrappedBackBuffers[i].As(&surface));
		THROW_IF_FAILED(mD2DDeviceContext->CreateBitmapFromDxgiSurface(
			surface.Get(),
			&bitmapProperties,
			&mBitmapRenderTargets[i]
		));
	}
}

void DXGIMgr::CreateDSV()
{
	mDefaultDs = ResourceMgr::I->CreateTexture("DefaultDepthStencil", mWindow.Width, mWindow.Height,
		DXGI_FORMAT_D24_UNORM_S8_UINT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_DEPTH_WRITE, Vec4{ 1.f });
	CreateDepthStencilView(mDefaultDs.get());
	CreateShaderResourceView(mDefaultDs.get(), DXGI_FORMAT_R24_UNORM_X8_TYPELESS);

	mShadowDs = ResourceMgr::I->CreateTexture("ShadowDepthStencil", mWindow.Width * 4, mWindow.Height * 4,
		DXGI_FORMAT_D24_UNORM_S8_UINT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_GENERIC_READ, Vec4{ 1.f });
	CreateDepthStencilView(mShadowDs.get());
	CreateShaderResourceView(mShadowDs.get(), DXGI_FORMAT_R24_UNORM_X8_TYPELESS);

	mCustomDs = ResourceMgr::I->CreateTexture("CustomDepthStencil", mWindow.Width, mWindow.Height,
		DXGI_FORMAT_D24_UNORM_S8_UINT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_GENERIC_READ, Vec4{ 0.f });
	CreateDepthStencilView(mCustomDs.get());
	CreateShaderResourceView(mCustomDs.get(), DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
}

void DXGIMgr::CreateMRTs()
{
#pragma region SwapChain
	{
		std::vector<RenderTarget> rts(mSwapChainBuffCnt);

		// 스왑 체인 후면 버퍼 생성
		for (UINT i = 0; i < mSwapChainBuffCnt; ++i) {
			ComPtr<ID3D12Resource> resource;
			mSwapChain->GetBuffer(i, IID_PPV_ARGS(&resource));

			std::string name = "SwapChainTarget_" + std::to_string(i);
			rts[i].Target = ResourceMgr::I->CreateTexture(name, resource);
		}

		mMRTs[static_cast<UINT8>(GroupType::SwapChain)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::SwapChain)]->Create(GroupType::SwapChain, std::move(rts), mDefaultDs);
	}
#pragma endregion

#pragma region Shadow
	{
		// 그림자 렌더 타겟 생성
		std::vector<RenderTarget> rts(0);
		mMRTs[static_cast<UINT8>(GroupType::Shadow)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::Shadow)]->Create(GroupType::Shadow, std::move(rts), mShadowDs);
	}
#pragma endregion

#pragma region CustomDepth
	{
		// 커스텀 깊이 버퍼 렌더 타겟 생성
		std::vector<RenderTarget> rts(0);
		mMRTs[static_cast<UINT8>(GroupType::CustomDepth)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::CustomDepth)]->Create(GroupType::CustomDepth, std::move(rts), mCustomDs);
	}
#pragma endregion

#pragma region GBuffer
	{
		std::vector<RenderTarget> rts(GBufferCount);

		rts[0].Target = ResourceMgr::I->CreateTexture("PositionTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[1].Target = ResourceMgr::I->CreateTexture("NormalTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);

		rts[2].Target = ResourceMgr::I->CreateTexture("DiffuseTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[3].Target = ResourceMgr::I->CreateTexture("EmissiveTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);

		rts[4].Target = ResourceMgr::I->CreateTexture("MetallicSmoothnessTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R8G8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[5].Target = ResourceMgr::I->CreateTexture("OcclusionTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R16_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[6].Target = ResourceMgr::I->CreateTexture("OutlineTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R16_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);

		mMRTs[static_cast<UINT8>(GroupType::GBuffer)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::GBuffer)]->Create(GroupType::GBuffer, std::move(rts), mDefaultDs);
	}
#pragma endregion

#pragma region Lighting
	{
		std::vector<RenderTarget> rts(LightingCount);

		rts[0].Target = ResourceMgr::I->CreateTexture("DiffuseAlbedoTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[1].Target = ResourceMgr::I->CreateTexture("SpecularAlbedoTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[2].Target = ResourceMgr::I->CreateTexture("AmbientTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		mMRTs[static_cast<UINT8>(GroupType::Lighting)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::Lighting)]->Create(GroupType::Lighting, std::move(rts), mDefaultDs);
	}
#pragma endregion

#pragma region OffScreen
	{
		std::vector<RenderTarget> rts(3);

		rts[0].Target = ResourceMgr::I->CreateTexture("OffScreenTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);

		rts[1].Target = RESOURCE<Texture>("NormalTarget");

		rts[2].Target = RESOURCE<Texture>("EmissiveTarget");

		mMRTs[static_cast<UINT8>(GroupType::OffScreen)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::OffScreen)]->Create(GroupType::OffScreen, std::move(rts), mDefaultDs);
	}
#pragma endregion

#pragma region SSAO
	{
		std::vector<RenderTarget> rts(SsaoCount);

		rts[0].Target = ResourceMgr::I->CreateTexture("SSAOTarget_0", static_cast<UINT>(mWindow.Width / 2.f), static_cast<UINT>(mWindow.Height / 2.f),
			DXGI_FORMAT_R16_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, Vec4{ 1.f });

		rts[1].Target = ResourceMgr::I->CreateTexture("SSAOTarget_1", static_cast<UINT>(mWindow.Width / 2.f), static_cast<UINT>(mWindow.Height / 2.f),
			DXGI_FORMAT_R16_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, Vec4{ 1.f });

		mMRTs[static_cast<UINT8>(GroupType::Ssao)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::Ssao)]->Create(GroupType::Ssao, std::move(rts), nullptr, Vec4{ 1.f });
	}
#pragma endregion

#pragma region Bloom
	{
		std::vector<RenderTarget> rts(BloomCount);

		std::array<float, BloomCount> samplingCount{ 1.f, 2.f, 2.f, 2.f, 1.f, 1.f, 1.f / 2.f, 1.f, 1.f , 1.f / 2.f, 1.f, 1.f };
		Vec2 targetSize = { (float)mWindow.Width, (float)mWindow.Height };

		for (int i = 0; i < BloomCount; ++i) {
			std::string name = "BloomTarget_" + std::to_string(i);

			// 마지막 블룸 텍스처는 최종 출력 텍스처
			if (i + 1 == BloomCount) {
				name = "BloomTarget";
			}

			targetSize /= samplingCount[i];

			rts[i].Target = ResourceMgr::I->CreateTexture(name, static_cast<UINT>(targetSize.x), static_cast<UINT>(targetSize.y),
				DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
		}

		mMRTs[static_cast<UINT8>(GroupType::Bloom)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::Bloom)]->Create(GroupType::Bloom, std::move(rts), nullptr);
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

	BOOL isFullScreenState = FALSE;
	mSwapChain->GetFullscreenState(&isFullScreenState, nullptr);
	mSwapChain->SetFullscreenState(!isFullScreenState, nullptr);

	DXGI_MODE_DESC dxgiTargetParameters{};
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = GetWindowWidth();
	dxgiTargetParameters.Height = GetWindowHeight();
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	mSwapChain->ResizeTarget(&dxgiTargetParameters);

	// TODO : 전체화면 오류 문제 사용중 해제로 인한
	for (int i = 0; i < mSwapChainBuffCnt; ++i) {
		mWrappedBackBuffers[i].Reset();
		mBitmapRenderTargets[i].Reset();
	}

	mD3D11On12Device.Reset();
	mD3D11DeviceContext.Reset();
	mD2DFactory.Reset();
	mDWriteFactory.Reset();
	mD2DDevice.Reset();
	mD2DDeviceContext.Reset();
	TextMgr::I->Reset();

	GetMRT(GroupType::SwapChain)->ReleaseRenderTargets();

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	mSwapChain->GetDesc(&swapChainDesc);
	THROW_IF_FAILED(mSwapChain->ResizeBuffers(
		mSwapChainBuffCnt,
		GetWindowWidth(),
		GetWindowHeight(),
		swapChainDesc.BufferDesc.Format,
		swapChainDesc.Flags)
	);

	mCurrBackBufferIdx = mSwapChain->GetCurrentBackBufferIndex();

	std::vector<RenderTarget> rts(mSwapChainBuffCnt);
	for (UINT i = 0; i < mSwapChainBuffCnt; ++i) {
		ComPtr<ID3D12Resource> resource;
		mSwapChain->GetBuffer(i, IID_PPV_ARGS(&resource));

		std::string name = "SwapChainTarget_" + std::to_string(i);
		rts[i].Target = ResourceMgr::I->CreateTexture(name, resource);
	}
	mMRTs[static_cast<UINT8>(GroupType::SwapChain)]->Create(GroupType::SwapChain, std::move(rts), mDefaultDs);

	CreateDirect2DDevice();
	TextMgr::I->CreateBrush();
	WaitForGpuComplete();
}

void DXGIMgr::CreateFilter()
{
	mBlurFilter = std::make_unique<BlurFilter>(GetWindowWidth(), GetWindowHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT);
	mBlurFilter->Create();

	mLUTFilter = std::make_unique<LUTFilter>(GetWindowWidth(), GetWindowHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT);
	mLUTFilter->Create();

	mSobelFilter = std::make_unique<SobelFilter>(GetWindowWidth(), GetWindowHeight());
	mSobelFilter->Create();

	mBloom = std::make_unique<Bloom>();
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
	mCurrBackBufferIdx = mSwapChain->GetCurrentBackBufferIndex();

	mFrameResourceMgr->GetCurrFrameResource()->Fence = ++mFenceValues;

	mCmdQueue->Signal(mFence.Get(), mFenceValues);
}

void DXGIMgr::BuildScene()
{
	mCrntScene->Build();
	MeshRenderer::BuildMeshes();

	mCmdList->Close();
	ID3D12CommandList* cmdsLists[] = { mCmdList.Get() };
	mCmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	WaitForGpuComplete();

	MeshRenderer::ReleaseUploadBuffers();
}
