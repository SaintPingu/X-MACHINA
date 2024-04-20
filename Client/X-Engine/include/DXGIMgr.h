#pragma once

#pragma region Define
#define DEVICE DXGIMgr::I->GetDevice()
#define CMD_LIST DXGIMgr::I->GetCmdList()
#define FRAME_RESOURCE_MGR DXGIMgr::I->GetFrameResourceMgr()
#pragma endregion

#pragma region ClassForwardDecl
class PostProcessingShader;
class FrameResourceMgr;
class MultipleRenderTarget;
class Texture;
class BlurFilter;
class LUTFilter;
class Ssao;
class DescriptorHeap;
class GraphicsRootSignature;
class ComputeRootSignature;
class ParticleRenderer;
struct PassConstants;
#pragma endregion

#pragma region EnumClass
enum class DrawOption {
	Main = 0,
	Debug,
};

class FilterOption : public DwordOverloader<FilterOption> {
	DWORD_OVERLOADER(FilterOption)

	static const DWORD None = 0x001;
	static const DWORD Blur = 0x002;
	static const DWORD Tone = 0x004;
	static const DWORD LUT = 0x008;
	static const DWORD Ssao = 0x010;
};
#pragma endregion

#pragma region Class
// device, swapchain 등 DXGI 전반 및 렌더링을 관리한다.
class DXGIMgr : public Singleton<DXGIMgr> {
	friend Singleton;

private:
	// window
	HINSTANCE	mInstance{};
	WindowInfo	mWindow{};

	// msaa
	bool		mIsMsaa4xEnabled = false;
	UINT		mMsaa4xQualityLevels{};

	// device
	ComPtr<IDXGIFactory4>				mFactory{};
	ComPtr<IDXGISwapChain3>				mSwapChain{};
	ComPtr<ID3D12Device>				mDevice{};

	// command
	ComPtr<ID3D12CommandAllocator>		mCmdAllocator{};
	ComPtr<ID3D12CommandQueue>			mCmdQueue{};
	ComPtr<ID3D12GraphicsCommandList>	mCmdList{};

	// swap chain
	static constexpr UINT				mSwapChainBuffCnt = 2;
	UINT								mCurrBackBufferIdx{};

	// frameResource
	uptr<FrameResourceMgr>				mFrameResourceMgr;

	// fence
	ComPtr<ID3D12Fence>					mFence{};
	UINT32								mFenceValues{};
	HANDLE								mFenceEvent{};

	// root signature
	sptr<GraphicsRootSignature>			mGraphicsRootSignature{};
	sptr<ComputeRootSignature>			mComputeRootSignature{};
	sptr<ComputeRootSignature>			mParticleComputeRootSignature{};

	// descriptor heap
	sptr<DescriptorHeap>				mDescriptorHeap{};

	// descriptor
	UINT								mCbvSrvUavDescriptorIncSize{};
	UINT								mRtvDescriptorIncSize{};
	UINT								mDsvDescriptorIncSize{};
	sptr<Texture>						mDefaultDs{};
	sptr<Texture>						mShadowDs{};

	// filter
	DWORD								mFilterOption{};
	uptr<BlurFilter>					mBlurFilter;
	uptr<LUTFilter>						mLUTFilter;

	// SSAO
	uptr<Ssao>							mSsao;

	D3D12_RESOURCE_BARRIER				mBarrier;

	// draw option
	DrawOption							mDrawOption{};

	// MRT
	std::array<sptr<MultipleRenderTarget>, MRTGroupTypeCount> mMRTs{};

protected:
#pragma region C/Dtor
	DXGIMgr();
	virtual ~DXGIMgr() = default;
#pragma endregion

public:
#pragma region Getter
	HWND GetHwnd() const									{ return mWindow.Hwnd; }
	short GetWindowWidth() const							{ return mWindow.Width; }
	short GetWindowHeight() const							{ return mWindow.Height; }
	RComPtr<ID3D12Device> GetDevice() const					{ return mDevice; }
	RComPtr<ID3D12GraphicsCommandList> GetCmdList() const	{ return mCmdList; }
	UINT GetCbvSrvUavDescriptorIncSize() const				{ return mCbvSrvUavDescriptorIncSize; }
	UINT GetRtvDescriptorIncSize() const					{ return mRtvDescriptorIncSize; }
	UINT GetDsvDescriptorIncSize() const					{ return mDsvDescriptorIncSize; }
	FrameResourceMgr* GetFrameResourceMgr() const			{ return mFrameResourceMgr.get(); }
	const auto& GetMRT(GroupType groupType) const			{ return mMRTs[static_cast<UINT8>(groupType)]; }
	const DWORD GetFilterOption() const						{ return mFilterOption; }
	rsptr<DescriptorHeap> GetDescHeap() const				{ return mDescriptorHeap; }
	Ssao* GetSsao() const									{ return mSsao.get(); }

	// [param]에 해당하는 root parameter index를 반환한다.
	UINT GetGraphicsRootParamIndex(RootParam param) const;
	UINT GetComputeRootParamIndex(RootParam param) const;
	UINT GetParticleComputeRootParamIndex(RootParam param) const;
	RComPtr<ID3D12RootSignature> GetGraphicsRootSignature() const;
	RComPtr<ID3D12RootSignature> GetComputeRootSignature() const;	
	RComPtr<ID3D12RootSignature> GetParticleComputeRootSignature() const;

#pragma endregion

#pragma region Setter
	void SetFilterOption(DWORD option)		{ mFilterOption = option; }
	void SetDrawOption(DrawOption option)	{ mDrawOption = option; }

	// [data]를 32BitConstants에 Set한다.
	void SetGraphicsRoot32BitConstants(RootParam param, const Matrix& data, UINT offset);
	void SetGraphicsRoot32BitConstants(RootParam param, const Vec4& data, UINT offset);
	void SetGraphicsRoot32BitConstants(RootParam param, float data, UINT offset);
	void SetGraphicsRoot32BitConstants(RootParam param, int data, UINT offset);

	// gpuAddr에 있는 CBV를 Set한다.
	void SetGraphicsRootConstantBufferView(RootParam param, D3D12_GPU_VIRTUAL_ADDRESS gpuAddr);

	// gpuAddr에 있는 SRV를 Set한다.
	void SetGraphicsRootShaderResourceView(RootParam param, D3D12_GPU_VIRTUAL_ADDRESS gpuAddr);
#pragma endregion

public:
	void Init(HINSTANCE hInstance, const WindowInfo& window);
	void Release();

	// buffer(DepthStencil, ...)의 SRV 리소스를 생성한다.
	void CreateShaderResourceView(Texture* texture, DXGI_FORMAT srvFormat);
	// texture의 SRV 리소스를 생성한다.
	void CreateShaderResourceView(Texture* texture);
	// texture의 UAV 리소스를 생성한다.
	void CreateUnorderedAccessView(Texture* texture);
	// texture의 DSV 리소스를 생성한다.
	void CreateDepthStencilView(Texture* texture);

	// exit
	void Terminate();

	// update dxgi
	void Update();

	// render scene
	void Render();

	// full screen on/off
	void ToggleFullScreen();

private:
	// reset command 
	void MainPassRenderBegin();
	void PostPassRenderBegin();

	// close command
	void RenderEnd();

	// device
	void CreateFactory();
	void CreateDevice();
	void SetMSAA();
	void CreateFence();
	void SetIncrementSize();

	void CreateDirect3DDevice();
	void CreateCmdQueueAndList();

	void CreateSwapChain();
	void CreateGraphicsRootSignature();
	void CreateComputeRootSignature();
	void CreateDescriptorHeaps(int cbvCount, int srvCount, int uavCount, int skyBoxSrvCount, int dsvCount);
	void CreateDSV();
	void CreateMRTs();
	void CreateFrameResources();

	// full screen on/off (resize swap chain buffer)
	void ChangeSwapChainState();
	void CreateFilter();
	void CreateSsao();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void BuildScene();
};
#pragma endregion