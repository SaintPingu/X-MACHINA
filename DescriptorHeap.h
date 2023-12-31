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
class DescriptorHeap
{
private:
	ComPtr<ID3D12DescriptorHeap> mHeap{};

	DescriptorHandle mCbvHandle{};
	DescriptorHandle mSrvHandle{};

public:
	DescriptorHeap()          = default;
	virtual ~DescriptorHeap() = default;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return mHeap->GetCPUDescriptorHandleForHeapStart(); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return mHeap->GetGPUDescriptorHandleForHeapStart(); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvStartHandle() const { return mCbvHandle.CpuStart; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvStartHandle() const { return mCbvHandle.GpuStart; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvStartHandle() const { return mSrvHandle.CpuStart; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvStartHandle() const { return mSrvHandle.GpuStart; }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvLastHandle() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvLastHandle() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvLastHandle() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvLastHandle() const;

public:
	void Create(int cbvCount, int srvCount);

	void CreateSrv(RComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc);
	void CreateSrvs(RComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc, UINT heapIndex);

	void Set();

private:
	void Close();
};
#pragma endregion