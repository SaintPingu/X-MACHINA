#pragma once

#pragma region Define
#define dxgi DXGIMgr::Inst()
#define device dxgi->GetDevice()
#define cmdList dxgi->GetCmdList()
#pragma endregion

#pragma region ClassForwardDecl
class PostProcessingShader;
#pragma endregion


#pragma region EnumClass
enum class DrawOption {
	Main = 0,
};
#pragma endregion


#pragma region Class
class DXGIMgr {
	SINGLETON_PATTERN(DXGIMgr)

private:
	// window
	HINSTANCE	mInstance{};
	HWND		mWnd{};

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
	static constexpr UINT mSwapChainBuffCnt{ 2 };
	static constexpr UINT mRtvCnt = 5;
	static constexpr std::array<DXGI_FORMAT, mRtvCnt> mRtvFormats{ DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };

	UINT mSwapChainBuffIdx{};
	ComPtr<ID3D12Resource> mSwapChainBuffers[mSwapChainBuffCnt]{};

	// view
	UINT							mCbvSrvDescriptorIncSize{};
	UINT							mRtvDescriptorIncSize{};
	ComPtr<ID3D12DescriptorHeap>	mRtvHeap{};
	ComPtr<ID3D12DescriptorHeap>	mDsvHeap{};
	ComPtr<ID3D12Resource>			mDepthStencilBuff{};
	D3D12_CPU_DESCRIPTOR_HANDLE		mDsvHandle{};

	// command
	ComPtr<ID3D12CommandAllocator>		mCmdAllocator{};
	ComPtr<ID3D12CommandQueue>			mCmdQueue{};
	ComPtr<ID3D12GraphicsCommandList>	mCmdList{};

	// fence
	ComPtr<ID3D12Fence> mFence{};
	UINT64				mFenceValues[mSwapChainBuffCnt]{};
	HANDLE				mFenceEvent{};

	D3D12_CPU_DESCRIPTOR_HANDLE	mSwapChainBuffRtvHandles[mSwapChainBuffCnt]{};

	sptr<PostProcessingShader>	mPostProcessingShader{};

	DrawOption mDrawOption{};

public:
#pragma region C/Dtor
	DXGIMgr();
	virtual ~DXGIMgr() = default;
#pragma endregion

#pragma region Getter
	RComPtr<ID3D12Device> GetDevice() const { return mDevice; }
	RComPtr<ID3D12GraphicsCommandList> GetCmdList() const { return mCmdList; }
	HWND GetHwnd() const { return mWnd; }
	const auto& GetRtvFormats() const { return mRtvFormats; }
	UINT GetCbvSrvDescriptorIncSize() const { return mCbvSrvDescriptorIncSize; }
	UINT GetRtvDescriptorIncSize() const { return mRtvDescriptorIncSize; }
#pragma endregion

#pragma region Setter
	void SetDrawOption(DrawOption option) { mDrawOption = option; }
#pragma endregion

public:
	void Init(HINSTANCE hInstance, HWND hMainWnd);
	void Release();
	void Terminate(); // 강제종료

	void Render();

	void ToggleFullScreen();

	void ClearDepth();
	void ClearStencil();
	void ClearDepthStencil();

private:
	void StartCommand();
	void StopCommand();

	void CreateFactory();
	void CreateDevice();
	void SetMSAA();
	void CreateFence();
	void SetIncrementSize();

	void CreateDirect3DDevice();
	void CreateCmdQueueAndList();
	void CreateRtvAndDsvDescriptorHeaps();

	void CreateSwapChain();
	void CreateRTVs();
	void CreateDSV();

	void ChangeSwapChainState();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void BuildObjects();
};
#pragma endregion