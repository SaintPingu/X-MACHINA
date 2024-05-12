#include "EnginePch.h"
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
#include "InputMgr.h"
#include "Component/ParticleSystem.h"

#pragma region Imgui
#include "../Imgui/ImGuiMgr.h"
#pragma endregion


DXGIMgr::DXGIMgr()
	:
	mDescriptorHeap(std::make_shared<DescriptorHeap>())
{
	DWORD filterOption = 0;
	filterOption |= FilterOption::None;
	//filterOption |= FilterOption::Blur;
	filterOption |= FilterOption::LUT;
	filterOption |= FilterOption::Tone;
	filterOption |= FilterOption::Ssao;
	filterOption |= FilterOption::Shadow;

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

	ResourceMgr::I->LoadResources();
	ParticleManager::I->Init();

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

	// ���� �������� ���� �Ҵ��ڸ� �����´�.
	auto& cmdAllocator = mFrameResourceMgr->GetCurrFrameResource()->CmdAllocator;
	cmdAllocator->Reset();
	mCmdList->Reset(cmdAllocator.Get(), NULL);

	MainPassRenderBegin();
}

void DXGIMgr::Render()
{
#pragma region MainRender
	// �ش� �Լ��� �ȿ��� �ڽ��� ����� ���� ���۸� Ŭ���� �Ѵ�.
	GetMRT(GroupType::SwapChain)->ClearRenderTargetView(mCurrBackBufferIdx, 1.f);
	GetMRT(GroupType::Shadow)->ClearRenderTargetView(1.f);
	GetMRT(GroupType::GBuffer)->ClearRenderTargetView(1.f);
	GetMRT(GroupType::Lighting)->ClearRenderTargetView(1.f);
	GetMRT(GroupType::OffScreen)->ClearRenderTargetView(0, 1.f);
	GetMRT(GroupType::CustomDepth)->ClearRenderTargetView(0.f);

	// �׸��� �� �ؽ�ó�� ���� Ÿ������ �����ϰ� �׸��� ������
	GetMRT(GroupType::Shadow)->OMSetRenderTargets(0, 0);
	Scene::I->RenderShadow();
	GetMRT(GroupType::Shadow)->WaitTargetToResource();

	// GBuffer�� ���� Ÿ������ �����ϰ� ���۵� ������
	GetMRT(GroupType::GBuffer)->OMSetRenderTargets();
	Scene::I->RenderDeferred();
	GetMRT(GroupType::GBuffer)->WaitTargetToResource();

	// deferred ������ ��ü�� ���ؼ��� ssao ����
	if (mFilterOption & FilterOption::Ssao)
		mSsao->Execute(1);

	// ����Ʈ �� �ؽ�ó�� ���� Ÿ������ �����ϰ� ����Ʈ ������
	GetMRT(GroupType::Lighting)->OMSetRenderTargets();
	Scene::I->RenderLights();
	GetMRT(GroupType::Lighting)->WaitTargetToResource();

	// Ŀ���� ���� ���� ������ 
	GetMRT(GroupType::CustomDepth)->OMSetRenderTargets(0, 0);
	Scene::I->RenderCustomDepth();
	GetMRT(GroupType::CustomDepth)->WaitTargetToResource();

	// �ĸ� ���۴�� ȭ�� �� �ؽ�ó�� ���� Ÿ������ �����ϰ� ������
	GetMRT(GroupType::GBuffer)->WaitResourceToTarget(static_cast<UINT8>(GBuffer::Normal));
	GetMRT(GroupType::OffScreen)->OMSetRenderTargets();
	Scene::I->RenderFinal();
	Scene::I->RenderForward();
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

	GetMRT(GroupType::SwapChain)->OMSetRenderTargets(1, mCurrBackBufferIdx);
	Scene::I->RenderPostProcessing(offScreenIndex);
	Scene::I->RenderUI();
	//ImGuiMgr::I->Render();
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
	Scene::I->ClearRenderedObjects();

	mDescriptorHeap->Set();

	// �׷��Ƚ� ���̴� ���� ����
	CMD_LIST->SetGraphicsRootSignature(GetGraphicsRootSignature().Get());
	CMD_LIST->SetGraphicsRootConstantBufferView(GetGraphicsRootParamIndex(RootParam::Ssao), FRAME_RESOURCE_MGR->GetSSAOCBGpuAddr());
	CMD_LIST->SetGraphicsRootShaderResourceView(GetGraphicsRootParamIndex(RootParam::Material), FRAME_RESOURCE_MGR->GetMatBufferGpuAddr());
	CMD_LIST->SetGraphicsRootDescriptorTable(GetGraphicsRootParamIndex(RootParam::Texture), mDescriptorHeap->GetGPUHandle());
	CMD_LIST->SetGraphicsRootDescriptorTable(GetGraphicsRootParamIndex(RootParam::SkyBox), mDescriptorHeap->GetSkyBoxGPUStartSrvHandle());

	// ��ƼŬ ��ǻƮ ���̴� ���� ����
	CMD_LIST->SetComputeRootSignature(GetParticleComputeRootSignature().Get());
	CMD_LIST->SetComputeRootShaderResourceView(GetParticleComputeRootParamIndex(RootParam::ParticleSystem), FRAME_RESOURCE_MGR->GetParticleSystemGpuAddr());
	CMD_LIST->SetComputeRootUnorderedAccessView(GetParticleComputeRootParamIndex(RootParam::ParticleShared), FRAME_RESOURCE_MGR->GetParticleSharedGpuAddr());
}

void DXGIMgr::PostPassRenderBegin()
{
	// imgui ���� �غ� �� ������Ʈ
	//ImGuiMgr::I->Render_Prepare();
	//ImGuiMgr::I->Update();

	// ����Ʈ ���μ��� ��ǻƮ ���̴� ���� ����
	CMD_LIST->SetComputeRootSignature(GetComputeRootSignature().Get());
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
	// client�� screen rect�� OS�κ��� �޾ƿ� �����Ѵ�.
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
	mDefaultDs = ResourceMgr::I->CreateTexture("DefaultDepthStencil", mWindow.Width, mWindow.Height,
		DXGI_FORMAT_D24_UNORM_S8_UINT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_DEPTH_WRITE, Vec4{1.f});
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

		// �ĸ� ���� ���ҽ��� MRT Ÿ�ٿ� �����Ѵ�. 
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
		// �׸��ڿ����� ���� ���۸� ����ϸ� ��� ���� Ÿ�ٵ� ������� �ʴ´�.
		std::vector<RenderTarget> rts(0);
		mMRTs[static_cast<UINT8>(GroupType::Shadow)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::Shadow)]->Create(GroupType::Shadow, std::move(rts), mShadowDs);
	}
#pragma endregion

#pragma region Shadow
	{
		// �׸��ڿ����� ���� ���۸� ����ϸ� ��� ���� Ÿ�ٵ� ������� �ʴ´�.
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
			DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[4].Target = ResourceMgr::I->CreateTexture("MetallicSmoothnessTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R8G8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		rts[5].Target = ResourceMgr::I->CreateTexture("OcclusionTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R16_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

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
		std::vector<RenderTarget> rts(2);

		rts[0].Target = ResourceMgr::I->CreateTexture("OffScreenTarget", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);

		rts[1].Target = RESOURCE<Texture>("NormalTarget");

		mMRTs[static_cast<UINT8>(GroupType::OffScreen)] = std::make_shared<MultipleRenderTarget>();
		mMRTs[static_cast<UINT8>(GroupType::OffScreen)]->Create(GroupType::OffScreen, std::move(rts), mDefaultDs);
	}
#pragma endregion

#pragma region SSAO
	{
		std::vector<RenderTarget> rts(SsaoCount);

		rts[0].Target = ResourceMgr::I->CreateTexture("SSAOTarget_0", mWindow.Width, mWindow.Height,
			DXGI_FORMAT_R16_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, Vec4{ 1.f });

		rts[1].Target = ResourceMgr::I->CreateTexture("SSAOTarget_1", mWindow.Width, mWindow.Height,
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

	// ���� Ÿ���� ��� �����Ѵ�.
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

	// �ؽ�ó�� MRT�� ���� �����Ѵ�.
	std::vector<RenderTarget> rts(mSwapChainBuffCnt);
	for (UINT i = 0; i < mSwapChainBuffCnt; ++i) {
		ComPtr<ID3D12Resource> resource;
		mSwapChain->GetBuffer(i, IID_PPV_ARGS(&resource));

		std::string name = "SwapChainTarget_" + std::to_string(i);
		rts[i].Target = ResourceMgr::I->CreateTexture(name, resource);
	}
	mMRTs[static_cast<UINT8>(GroupType::SwapChain)]->Create(GroupType::SwapChain, std::move(rts), mDefaultDs);
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
	// ���� ���������� �Ѿ �� ����ȭ�� ���� �ʴ´�.
	mCurrBackBufferIdx = mSwapChain->GetCurrentBackBufferIndex();

	mFrameResourceMgr->GetCurrFrameResource()->Fence = ++mFenceValues;

	mCmdQueue->Signal(mFence.Get(), mFenceValues);
}

void DXGIMgr::BuildScene()
{
	Scene::I->BuildObjects();

	mCmdList->Close();
	ID3D12CommandList* cmdsLists[] = { mCmdList.Get() };
	mCmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// GPU�� ��� ������ ���� �� ���ε� ���۸� �����ؾ� �Ѵ�.
	WaitForGpuComplete();

	Scene::I->ReleaseUploadBuffers();
}
