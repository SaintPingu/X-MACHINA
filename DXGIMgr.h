#pragma once

#define DRAW_SCENE_COLOR				'S'

#define DRAW_SCENE_TEXTURE				'T'
#define DRAW_SCENE_LIGHTING				'L'
#define DRAW_SCENE_NORMAL				'N'
#define DRAW_SCENE_Z_DEPTH				'Z'
#define DRAW_SCENE_DEPTH				'D'

class CPostProcessingShader;

class DXGIMgr {
	SINGLETON_PATTERN(DXGIMgr)

private:
	// window
	HINSTANCE mInstance{};
	HWND mWnd{};

	// screen
	int mClientWidth{};
	int mClientHeight{};

	// device
	ComPtr<IDXGIFactory4> mFactory{};
	ComPtr<IDXGISwapChain3> mSwapChain{};
	ComPtr<ID3D12Device> mDevice{};

	bool mIsMsaa4xEnabled{ false };
	UINT mMsaa4xQualityLevels{};

	// swap chain
	static const UINT mSwapChainBufferCount{ 2 };
	UINT mSwapChainBufferIndex{};
	ComPtr<ID3D12Resource> mSwapChainBackBuffers[mSwapChainBufferCount]{};

	// RTV & DSV
	ComPtr<ID3D12DescriptorHeap> mRtvHeap{};

	ComPtr<ID3D12Resource> mDepthStencilBuffer{};
	ComPtr<ID3D12DescriptorHeap> mDsvHeap{};
	D3D12_CPU_DESCRIPTOR_HANDLE mDsvHandle{};

	// command
	ComPtr<ID3D12CommandAllocator> mCmdAllocator{};
	ComPtr<ID3D12CommandQueue> mCmdQueue{};
	ComPtr<ID3D12GraphicsCommandList> mCmdList{};

	// fence
	ComPtr<ID3D12Fence> mFence{};
	UINT64 mFenceValues[mSwapChainBufferCount]{};
	HANDLE mFenceEvent{};


	// others
#if defined(_DEBUG)
	ComPtr<ID3D12Debug> mDebugController{};
#endif

	D3D12_CPU_DESCRIPTOR_HANDLE	mSwapChainBackBufferRtvHandles[mSwapChainBufferCount]{};
	sptr<CPostProcessingShader> mPostProcessingShader{};
	int mDrawOption{ DRAW_SCENE_COLOR };

public:
	DXGIMgr();
	~DXGIMgr() = default;

	RComPtr<ID3D12Device> GetDevice() const { return mDevice; }
	RComPtr<ID3D12GraphicsCommandList> GetCmdList() const { return mCmdList; }
	HWND GetHwnd() const { return mWnd; }

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();
	void Terminate(); // 강제종료

private:
	/* Creation */
	// for CreateDirect3DDevice
	void CreateDirect3DDevice();
	void CreateFactory();
	void CreateDevice();
	void SetMSAA();
	void CreateFence();
	void SetIncrementSize();
	//

	void CreateCommandQueueAndList();
	void CreateRtvAndDsvDescriptorHeaps();

	void CreateSwapChain();
	void CreateRenderTargetViews();
	void CreateDepthStencilView();


	/* for rendering */
	void ChangeSwapChainState();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void StartCommand();
	void StopCommand();

public:
	void BuildObjects();
	void Render();

	void ToggleFullScreen();
	void SetDrawOption(int option);

	void ClearStencil();
};

inline RComPtr<ID3D12Device> GetDXGIDevice() { return DXGIMgr::Inst()->GetDevice(); }
inline RComPtr<ID3D12GraphicsCommandList> GetDXGICmdList() { return DXGIMgr::Inst()->GetCmdList(); }

#define dxgi DXGIMgr::Inst()
#define device dxgi->GetDevice()
#define cmdList dxgi->GetCmdList()