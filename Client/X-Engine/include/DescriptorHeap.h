#pragma once


#pragma region Struct
struct DescriptorHandle {
	D3D12_CPU_DESCRIPTOR_HANDLE CpuStart{};
	D3D12_CPU_DESCRIPTOR_HANDLE CpuNext{};

	D3D12_GPU_DESCRIPTOR_HANDLE GpuStart{};
	D3D12_GPU_DESCRIPTOR_HANDLE GpuNext{};
};
#pragma endregion


#pragma region Class
// util of ID3D12DescriptorHeap
class DescriptorHeap {
private:
	ComPtr<ID3D12DescriptorHeap> mCbvSrvUavHeap{};
	ComPtr<ID3D12DescriptorHeap> mDsvHeap{};

	DescriptorHandle mCbvHandle{};
	DescriptorHandle mSrvHandle{};
	DescriptorHandle mUavHandle{};
	DescriptorHandle mSkyBoxSrvHandle{};
	DescriptorHandle mDsvHandle{};

public:
	DescriptorHeap()          = default;
	virtual ~DescriptorHeap() = default;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const		  { return mCbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const		  { return mCbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart(); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvStartHandle()  const { return mCbvHandle.CpuStart; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvStartHandle()  const { return mCbvHandle.GpuStart; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvStartHandle()  const { return mSrvHandle.CpuStart; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvStartHandle()  const { return mSrvHandle.GpuStart; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUUavStartHandle()  const { return mUavHandle.CpuStart; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUUavStartHandle()  const { return mUavHandle.GpuStart; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetSkyBoxCPUStartSrvHandle() const { return mSkyBoxSrvHandle.CpuStart; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetSkyBoxGPUStartSrvHandle() const { return mSkyBoxSrvHandle.GpuStart; }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvLastHandle() const;		// CPU의 CBV 마지막 주소 핸들값을 반환한다. (next 이전값)
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvLastHandle() const;		// GPU의 CBV 마지막 주소 핸들값을 반환한다. (next 이전값)
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvLastHandle() const;		
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvLastHandle() const;		
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUUavLastHandle() const;		
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUUavLastHandle() const;		
	D3D12_CPU_DESCRIPTOR_HANDLE GetSkyBoxCPUSrvLastHandle() const;	
	D3D12_GPU_DESCRIPTOR_HANDLE GetSkyBoxGPUSrvLastHandle() const;	
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDsvLastHandle() const;	    
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDsvLastHandle() const;		

	UINT GetGPUCbvLastHandleIndex() const; 
	UINT GetGPUSrvLastHandleIndex() const; 
	UINT GetGPUUavLastHandleIndex() const; 
	UINT GetSkyBoxGPUSrvLastHandleIndex() const; 

public:
	// descriptor heap을 생성하고 핸들값들을 설정한다.
	void Create(int cbvCount, int srvCount, int uavCount, int skyBoxSrvCount, int dsvCount);

	void CreateShaderResourceView(RComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc);
	void CreateUnorderedAccessView(RComPtr<ID3D12Resource> resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc);
	void CreateDepthStencilView(RComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC* dsvDesc);

	// descriptor heap을 CommandList에 set한다.
	void Set();
};
#pragma endregion