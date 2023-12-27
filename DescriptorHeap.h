#pragma once


struct DESCRIPTOR_HANDLE {
	D3D12_CPU_DESCRIPTOR_HANDLE cpuStart{};
	D3D12_CPU_DESCRIPTOR_HANDLE cpuNext{};

	D3D12_GPU_DESCRIPTOR_HANDLE gpuStart{};
	D3D12_GPU_DESCRIPTOR_HANDLE gpuNext{};
};


class CDescriptorHeap
{
private:
	void Close();

public:
	CDescriptorHeap() = default;
	~CDescriptorHeap() = default;

	ComPtr<ID3D12DescriptorHeap> mHeap{};

	DESCRIPTOR_HANDLE mCbvHandle{};
	DESCRIPTOR_HANDLE mSrvHandle{};

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() { return(mHeap->GetCPUDescriptorHandleForHeapStart()); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() { return(mHeap->GetGPUDescriptorHandleForHeapStart()); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvStartHandle() { return(mCbvHandle.cpuStart); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvStartHandle() { return(mCbvHandle.gpuStart); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvStartHandle() { return(mSrvHandle.cpuStart); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvStartHandle() { return(mSrvHandle.gpuStart); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvLastHandle();
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvLastHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvLastHandle();
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvLastHandle();

	void Create(int cbvCount, int srvCount);

	void CreateSrv(RComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc);
	void CreateSrvs(RComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc, UINT heapIndex);

	void Set();
};
