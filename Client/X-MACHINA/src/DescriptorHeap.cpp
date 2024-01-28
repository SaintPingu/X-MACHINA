#include "stdafx.h"
#include "DescriptorHeap.h"
#include "DXGIMgr.h"


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUCbvLastHandle() const
{
	return { mCbvHandle.CpuNext.ptr - dxgi->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUCbvLastHandle() const
{
	return { mCbvHandle.GpuNext.ptr - dxgi->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUSrvLastHandle() const
{
	return { mSrvHandle.CpuNext.ptr - dxgi->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUSrvLastHandle() const
{
	return { mSrvHandle.GpuNext.ptr - dxgi->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUUavLastHandle() const
{
	return { mUavHandle.CpuNext.ptr - dxgi->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUUavLastHandle() const
{
	return { mUavHandle.GpuNext.ptr - dxgi->GetCbvSrvUavDescriptorIncSize() };
}
UINT DescriptorHeap::GetGPUCbvLastHandleIndex() const
{
	return { static_cast<UINT>((GetGPUCbvLastHandle().ptr - mCbvHandle.GpuStart.ptr) / dxgi->GetCbvSrvUavDescriptorIncSize()) };
}
UINT DescriptorHeap::GetGPUSrvLastHandleIndex() const
{
	return { static_cast<UINT>((GetGPUSrvLastHandle().ptr - mSrvHandle.GpuStart.ptr) / dxgi->GetCbvSrvUavDescriptorIncSize()) };
}

UINT DescriptorHeap::GetGPUUavLastHandleIndex() const
{
	return 0;
}


void DescriptorHeap::Create(int cbvCount, int srvCount, int uavCount)
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.NumDescriptors = cbvCount + srvCount + uavCount; // CBVs + SRVs + UAVs
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&mHeap));
	AssertHResult(hResult);

	// CBV CPU/GPU Descriptor�� ���� �ڵ� �ּҸ� �޾ƿ´�.
	mCbvHandle.CpuStart = mHeap->GetCPUDescriptorHandleForHeapStart();
	mCbvHandle.GpuStart = mHeap->GetGPUDescriptorHandleForHeapStart();

	// SRV�� CPU/GPU Descriptor�� ���� �ڵ� �ּҴ� CBV Descriptor�� �ں��� �����Ѵ�.
	mSrvHandle.CpuStart.ptr = mCbvHandle.CpuStart.ptr + (dxgi->GetCbvSrvUavDescriptorIncSize() * cbvCount);
	mSrvHandle.GpuStart.ptr = mCbvHandle.GpuStart.ptr + (dxgi->GetCbvSrvUavDescriptorIncSize() * cbvCount);

	// UAV�� CPU/GPU Descriptor�� ���� �ڵ� �ּҴ� SRV Descriptor�� �ں��� �����Ѵ�.
	mUavHandle.CpuStart.ptr = mSrvHandle.CpuStart.ptr + (dxgi->GetCbvSrvUavDescriptorIncSize() * srvCount);
	mUavHandle.GpuStart.ptr = mSrvHandle.GpuStart.ptr + (dxgi->GetCbvSrvUavDescriptorIncSize() * srvCount);

	// CPU/GPU�� Next�� Start�� ��ġ�� �����Ѵ�.
	mCbvHandle.CpuNext = mCbvHandle.CpuStart;
	mCbvHandle.GpuNext = mCbvHandle.GpuStart;
	mSrvHandle.CpuNext = mSrvHandle.CpuStart;
	mSrvHandle.GpuNext = mSrvHandle.GpuStart;
	mUavHandle.CpuNext = mUavHandle.CpuStart;
	mUavHandle.GpuNext = mUavHandle.GpuStart;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::CreateShaderResourceView(RComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc)
{
	// [resource]�� SRV�� [srvDesc]�� ���� �����Ѵ�.
	device->CreateShaderResourceView(resource.Get(), srvDesc, mSrvHandle.CpuNext);

	// SRV CPU/GPU�� Next�� ���� ��ġ�� ������Ų��.
	mSrvHandle.CpuNext.ptr += dxgi->GetCbvSrvUavDescriptorIncSize();
	mSrvHandle.GpuNext.ptr += dxgi->GetCbvSrvUavDescriptorIncSize();

	// ���ҽ��� SRV�� ��ġ�� ��ȯ�Ѵ�.
	return GetGPUSrvLastHandle();
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::CreateUnorderedAccessView(RComPtr<ID3D12Resource> resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc)
{
	// [resource]�� UAV�� [uavDesc]�� ���� �����Ѵ�.
	device->CreateUnorderedAccessView(resource.Get(), nullptr, uavDesc, mUavHandle.CpuNext);

	// UAV CPU/GPU�� Next�� ���� ��ġ�� ������Ų��.
	mUavHandle.CpuNext.ptr += dxgi->GetCbvSrvUavDescriptorIncSize();
	mUavHandle.GpuNext.ptr += dxgi->GetCbvSrvUavDescriptorIncSize();

	// ���ҽ��� UAV�� ��ġ�� ��ȯ�Ѵ�.
	return GetGPUUavLastHandle();
}

void DescriptorHeap::Set()
{
	cmdList->SetDescriptorHeaps(1, mHeap.GetAddressOf());
}