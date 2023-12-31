#include "stdafx.h"
#include "DescriptorHeap.h"
#include "DXGIMgr.h"


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUCbvLastHandle() const
{
	return { mCbvHandle.CpuNext.ptr - dxgi->GetCbvSrvDescriptorIncSize() };
}
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUCbvLastHandle() const
{
	return { mCbvHandle.GpuNext.ptr - dxgi->GetCbvSrvDescriptorIncSize() };
}
D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUSrvLastHandle() const
{
	return { mSrvHandle.CpuNext.ptr - dxgi->GetCbvSrvDescriptorIncSize() };
}
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUSrvLastHandle() const
{
	return { mSrvHandle.GpuNext.ptr - dxgi->GetCbvSrvDescriptorIncSize() };
}


void DescriptorHeap::Create(int cbvCount, int srvCount)
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc;
	descriptorHeapDesc.NumDescriptors = cbvCount + srvCount; //CBVs + SRVs
	descriptorHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorHeapDesc.NodeMask       = 0;
	HRESULT hResult                   = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&mHeap));
	AssertHResult(hResult);

	mCbvHandle.CpuStart = mHeap->GetCPUDescriptorHandleForHeapStart();
	mCbvHandle.GpuStart = mHeap->GetGPUDescriptorHandleForHeapStart();

	mSrvHandle.CpuStart.ptr = mCbvHandle.CpuStart.ptr + (dxgi->GetCbvSrvDescriptorIncSize() * cbvCount);
	mSrvHandle.GpuStart.ptr = mCbvHandle.GpuStart.ptr + (dxgi->GetCbvSrvDescriptorIncSize() * cbvCount);

	mCbvHandle.CpuNext = mCbvHandle.CpuStart;
	mCbvHandle.GpuNext = mCbvHandle.GpuStart;
	mSrvHandle.CpuNext = mSrvHandle.CpuStart;
	mSrvHandle.GpuNext = mSrvHandle.GpuStart;
}

void DescriptorHeap::CreateSrv(RComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc)
{
	device->CreateShaderResourceView(resource.Get(), srvDesc, mSrvHandle.CpuNext);
	Close();
}

void DescriptorHeap::CreateSrvs(RComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc, UINT heapIndex)
{
	mSrvHandle.CpuNext.ptr += (dxgi->GetCbvSrvDescriptorIncSize() * heapIndex);
	mSrvHandle.GpuNext.ptr += (dxgi->GetCbvSrvDescriptorIncSize() * heapIndex);
	CreateSrv(resource, srvDesc);
}

void DescriptorHeap::Set()
{
	cmdList->SetDescriptorHeaps(1, mHeap.GetAddressOf());
}


void DescriptorHeap::Close()
{
	mSrvHandle.CpuNext.ptr += dxgi->GetCbvSrvDescriptorIncSize();
	mSrvHandle.GpuNext.ptr += dxgi->GetCbvSrvDescriptorIncSize();
}