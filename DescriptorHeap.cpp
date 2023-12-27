#include "stdafx.h"
#include "DescriptorHeap.h"
#include "DXGIMgr.h"


D3D12_CPU_DESCRIPTOR_HANDLE CDescriptorHeap::GetCPUCbvLastHandle()
{
	D3D12_CPU_DESCRIPTOR_HANDLE lastHandle{};
	lastHandle.ptr = mCbvHandle.cpuNext.ptr - ::gnCbvSrvDescriptorIncrementSize;
	return lastHandle;
}
D3D12_GPU_DESCRIPTOR_HANDLE CDescriptorHeap::GetGPUCbvLastHandle()
{
	D3D12_GPU_DESCRIPTOR_HANDLE lastHandle{};
	lastHandle.ptr = mCbvHandle.gpuNext.ptr - ::gnCbvSrvDescriptorIncrementSize;
	return lastHandle;
}
D3D12_CPU_DESCRIPTOR_HANDLE CDescriptorHeap::GetCPUSrvLastHandle()
{
	D3D12_CPU_DESCRIPTOR_HANDLE lastHandle{};
	lastHandle.ptr = mSrvHandle.cpuNext.ptr - ::gnCbvSrvDescriptorIncrementSize;
	return lastHandle;
}
D3D12_GPU_DESCRIPTOR_HANDLE CDescriptorHeap::GetGPUSrvLastHandle()
{
	D3D12_GPU_DESCRIPTOR_HANDLE lastHandle{};
	lastHandle.ptr = mSrvHandle.gpuNext.ptr - ::gnCbvSrvDescriptorIncrementSize;
	return lastHandle;
}


void CDescriptorHeap::Create(int cbvCount, int srvCount)
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc;
	descriptorHeapDesc.NumDescriptors = cbvCount + srvCount; //CBVs + SRVs 
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&mHeap));
	assert(SUCCEEDED(hResult));

	mCbvHandle.cpuStart = mHeap->GetCPUDescriptorHandleForHeapStart();
	mCbvHandle.gpuStart = mHeap->GetGPUDescriptorHandleForHeapStart();

	mSrvHandle.cpuStart.ptr = mCbvHandle.cpuStart.ptr + (::gnCbvSrvDescriptorIncrementSize * cbvCount);
	mSrvHandle.gpuStart.ptr = mCbvHandle.gpuStart.ptr + (::gnCbvSrvDescriptorIncrementSize * cbvCount);

	mCbvHandle.cpuNext = mCbvHandle.cpuStart;
	mCbvHandle.gpuNext = mCbvHandle.gpuStart;
	mSrvHandle.cpuNext = mSrvHandle.cpuStart;
	mSrvHandle.gpuNext = mSrvHandle.gpuStart;
}

void CDescriptorHeap::CreateSrv(RComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc)
{
	device->CreateShaderResourceView(resource.Get(), srvDesc, mSrvHandle.cpuNext);
	Close();
}

void CDescriptorHeap::CreateSrvs(RComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc, UINT heapIndex)
{
	mSrvHandle.cpuNext.ptr += (::gnCbvSrvDescriptorIncrementSize * heapIndex);
	mSrvHandle.gpuNext.ptr += (::gnCbvSrvDescriptorIncrementSize * heapIndex);
	CreateSrv(resource, srvDesc);
}

void CDescriptorHeap::Close()
{
	mSrvHandle.cpuNext.ptr += ::gnCbvSrvDescriptorIncrementSize;
	mSrvHandle.gpuNext.ptr += ::gnCbvSrvDescriptorIncrementSize;
}

void CDescriptorHeap::Set()
{
	cmdList->SetDescriptorHeaps(1, mHeap.GetAddressOf());
}