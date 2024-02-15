#pragma once

#pragma region Define
#define dxgi DXGIMgr::Inst()
#define device dxgi->GetDevice()
#define cmdList dxgi->GetCmdList()
#define frmResMgr dxgi->GetFrameResourceMgr()
#pragma endregion

#pragma region ClassForwardDecl
class PostProcessingShader;
class FrameResourceMgr;
class MultipleRenderTarget;
class BlurFilter;
class LUTFilter;

struct PassConstants;
#pragma endregion

#pragma region EnumClass
enum class DrawOption {
	Main = 0,
	Texture,
	Normal,
	Depth,
};

enum class FilterOption : DWORD {
	None = 0x01,
	Blur = 0x02,
	Tone = 0x04,
	LUT	 = 0x08,
};
#pragma endregion


#pragma region Class
// device, swapchain 등 DXGI 전반 및 렌더링을 관리한다.
class DXGIMgr : public Singleton<DXGIMgr> {
	friend Singleton<DXGIMgr>;

private:
	// window
	HINSTANCE	mInstance{};
	HWND		mWnd{};			// 메인 윈도우 핸들

	// screen
	int mClientWidth{};
	int mClientHeight{};

	// device
	ComPtr<IDXGIFactory4>	mFactory{};
	ComPtr<IDXGISwapChain3> mSwapChain{};
	ComPtr<ID3D12Device>	mDevice{};

	bool mIsMsaa4xEnabled{ false };
	UINT mMsaa4xQualityLevels{};

	// swap chain
	static constexpr UINT mSwapChainBuffCnt = 2;
	UINT mCurrBackBufferIdx{}; // current swap chain buffer index

	// view (descriptor)
	UINT mCbvSrvUavDescriptorIncSize{};
	UINT mRtvDescriptorIncSize{};
	ComPtr<ID3D12DescriptorHeap>	mDsvHeap{};
	ComPtr<ID3D12Resource>			mDepthStencilBuff{};
	D3D12_CPU_DESCRIPTOR_HANDLE		mDsvHandle{};

	// command
	ComPtr<ID3D12CommandAllocator>		mCmdAllocator{};
	ComPtr<ID3D12CommandQueue>			mCmdQueue{};
	ComPtr<ID3D12GraphicsCommandList>	mCmdList{};

	// fence
	// fence array -> single object
	ComPtr<ID3D12Fence>						mFence{};
	UINT32									mFenceValues{};
	HANDLE									mFenceEvent{};

	// frameResource
	uptr<FrameResourceMgr>		mFrameResourceMgr;

	// MRT
	std::array<sptr<MultipleRenderTarget>, MRTGroupTypeCount>	mMRTs{};

	// filter
	DWORD				mFilterOption{};
	uptr<BlurFilter>	mBlurFilter;
	uptr<LUTFilter>		mLUTFilter;

	DrawOption		mDrawOption{};

protected:
#pragma region C/Dtor
	DXGIMgr();
	virtual ~DXGIMgr() = default;
#pragma endregion

public:
#pragma region Getter
	HWND GetHwnd() const									{ return mWnd; }
	RComPtr<ID3D12Device> GetDevice() const					{ return mDevice; }
	RComPtr<ID3D12GraphicsCommandList> GetCmdList() const	{ return mCmdList; }
	UINT GetCbvSrvUavDescriptorIncSize() const				{ return mCbvSrvUavDescriptorIncSize; }
	UINT GetRtvDescriptorIncSize() const					{ return mRtvDescriptorIncSize; }
	FrameResourceMgr* GetFrameResourceMgr() const			{ return mFrameResourceMgr.get(); }
	const auto& GetMRT(GroupType groupType) const			{ return mMRTs[static_cast<UINT8>(groupType)]; }
	const DWORD GetFilterOption() const						{ return mFilterOption; }
#pragma endregion

#pragma region Setter
	void SetDrawOption(DrawOption option) { mDrawOption = option; }
#pragma endregion

public:
	void Init(HINSTANCE hInstance, HWND hMainWnd);
	void Release();

	// 강제종료
	void Terminate();

	// update dxgi
	void Update();

	// render scene
	void Render();

	// full screen on/off
	void ToggleFullScreen();

	void ClearDepth();
	void ClearStencil();
	void ClearDepthStencil();

private:
	// reset command 
	// rename StartCommand -> RenderBegin
	void RenderBegin();

	// close command
	// rename StopCommand -> RenderEnd
	void RenderEnd();

	// for CreateDirect3DDevice
	void CreateFactory();
	void CreateDevice();
	void SetMSAA();
	void CreateFence();
	void SetIncrementSize();

	void CreateDirect3DDevice();
	void CreateCmdQueueAndList();
	void CreateRtvAndDsvDescriptorHeaps();

	void CreateSwapChain();
	void CreateDSV();
	void CreateMRTs();

	void CreateFrameResources();

	// full screen on/off (resize swap chain buffer)
	void ChangeSwapChainState();

	void CreateFilter();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void BuildScene();

};
#pragma endregion