#include "EnginePch.h"
#include "DescriptorHeap.h"
#include "DXGIMgr.h"


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUCbvLastHandle() const
{
	return { mCbvHandle.CpuNext.ptr - DXGIMgr::I->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUCbvLastHandle() const
{
	return { mCbvHandle.GpuNext.ptr - DXGIMgr::I->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUSrvLastHandle() const
{
	return { mSrvHandle.CpuNext.ptr - DXGIMgr::I->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUSrvLastHandle() const
{
	return { mSrvHandle.GpuNext.ptr - DXGIMgr::I->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUUavLastHandle() const
{
	return { mUavHandle.CpuNext.ptr - DXGIMgr::I->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUUavLastHandle() const
{
	return { mUavHandle.GpuNext.ptr - DXGIMgr::I->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetSkyBoxCPUSrvLastHandle() const
{
	return { mSkyBoxSrvHandle.CpuNext.ptr - DXGIMgr::I->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetSkyBoxGPUSrvLastHandle() const
{
	return { mSkyBoxSrvHandle.GpuNext.ptr - DXGIMgr::I->GetCbvSrvUavDescriptorIncSize() };
}
D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUDsvLastHandle() const
{
	return { mDsvHandle.CpuNext.ptr - DXGIMgr::I->GetDsvDescriptorIncSize() };
}
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUDsvLastHandle() const
{
	return { mDsvHandle.GpuNext.ptr - DXGIMgr::I->GetDsvDescriptorIncSize() };
}
UINT DescriptorHeap::GetGPUCbvLastHandleIndex() const
{
	return { static_cast<UINT>((GetGPUCbvLastHandle().ptr - mCbvHandle.GpuStart.ptr) / DXGIMgr::I->GetCbvSrvUavDescriptorIncSize()) };
}
UINT DescriptorHeap::GetGPUSrvLastHandleIndex() const
{
	return { static_cast<UINT>((GetGPUSrvLastHandle().ptr - mSrvHandle.GpuStart.ptr) / DXGIMgr::I->GetCbvSrvUavDescriptorIncSize()) };
}
UINT DescriptorHeap::GetGPUUavLastHandleIndex() const
{
	return { static_cast<UINT>((GetGPUUavLastHandle().ptr - mUavHandle.GpuStart.ptr) / DXGIMgr::I->GetCbvSrvUavDescriptorIncSize()) };
}
UINT DescriptorHeap::GetSkyBoxGPUSrvLastHandleIndex() const
{
	return { static_cast<UINT>((GetSkyBoxGPUSrvLastHandle().ptr - mSkyBoxSrvHandle.GpuStart.ptr) / DXGIMgr::I->GetCbvSrvUavDescriptorIncSize()) };
}

void DescriptorHeap::Create(int cbvCount, int srvCount, int uavCount, int skyBoxSrvCount, int dsvCount)
{
#pragma region CbvSrvUav
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
		descriptorHeapDesc.NumDescriptors = cbvCount + srvCount + uavCount + skyBoxSrvCount; // CBVs + SRVs + UAVs + SkyBoxSRVs
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descriptorHeapDesc.NodeMask = 0;
		HRESULT hResult = DEVICE->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&mCbvSrvUavHeap));
		AssertHResult(hResult);

		// CBV CPU/GPU Descriptor의 시작 핸들 주소를 받아온다.
		mCbvHandle.CpuStart = mCbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
		mCbvHandle.GpuStart = mCbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart();

		// SRV의 CPU/GPU Descriptor의 시작 핸들 주소는 CBV Descriptor의 뒤부터 시작한다.
		mSrvHandle.CpuStart.ptr = mCbvHandle.CpuStart.ptr + (DXGIMgr::I->GetCbvSrvUavDescriptorIncSize() * cbvCount);
		mSrvHandle.GpuStart.ptr = mCbvHandle.GpuStart.ptr + (DXGIMgr::I->GetCbvSrvUavDescriptorIncSize() * cbvCount);

		// UAV의 CPU/GPU Descriptor의 시작 핸들 주소는 SRV Descriptor의 뒤부터 시작한다.
		mUavHandle.CpuStart.ptr = mSrvHandle.CpuStart.ptr + (DXGIMgr::I->GetCbvSrvUavDescriptorIncSize() * srvCount);
		mUavHandle.GpuStart.ptr = mSrvHandle.GpuStart.ptr + (DXGIMgr::I->GetCbvSrvUavDescriptorIncSize() * srvCount);

		mSkyBoxSrvHandle.CpuStart.ptr = mUavHandle.CpuStart.ptr + (DXGIMgr::I->GetCbvSrvUavDescriptorIncSize() * uavCount);
		mSkyBoxSrvHandle.GpuStart.ptr = mUavHandle.GpuStart.ptr + (DXGIMgr::I->GetCbvSrvUavDescriptorIncSize() * uavCount);

		// CPU/GPU의 Next를 Start의 위치로 설정한다.
		mCbvHandle.CpuNext = mCbvHandle.CpuStart;
		mCbvHandle.GpuNext = mCbvHandle.GpuStart;
		mSrvHandle.CpuNext = mSrvHandle.CpuStart;
		mSrvHandle.GpuNext = mSrvHandle.GpuStart;
		mUavHandle.CpuNext = mUavHandle.CpuStart;
		mUavHandle.GpuNext = mUavHandle.GpuStart;
		mSkyBoxSrvHandle.CpuNext = mSkyBoxSrvHandle.CpuStart;
		mSkyBoxSrvHandle.GpuNext = mSkyBoxSrvHandle.GpuStart;
	}
#pragma endregion

#pragma region Dsv
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
		descriptorHeapDesc.NumDescriptors = dsvCount;
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDesc.NodeMask = 0;
		HRESULT hResult = DEVICE->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&mDsvHeap));
		AssertHResult(hResult);

		mDsvHandle.CpuStart = mDsvHeap->GetCPUDescriptorHandleForHeapStart();
		mDsvHandle.GpuStart = mDsvHeap->GetGPUDescriptorHandleForHeapStart();

		mDsvHandle.CpuNext = mDsvHandle.CpuStart;
		mDsvHandle.GpuNext = mDsvHandle.GpuStart;
	}
#pragma endregion
}

void DescriptorHeap::CreateShaderResourceView(RComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc)
{
	// TextureCube일 경우 따로 GPU 핸들에 저장한다.
	if (srvDesc->ViewDimension == D3D12_SRV_DIMENSION_TEXTURECUBE) {
		DEVICE->CreateShaderResourceView(resource.Get(), srvDesc, mSkyBoxSrvHandle.CpuNext);
		mSkyBoxSrvHandle.CpuNext.ptr += DXGIMgr::I->GetCbvSrvUavDescriptorIncSize();
		mSkyBoxSrvHandle.GpuNext.ptr += DXGIMgr::I->GetCbvSrvUavDescriptorIncSize();
	}
	else {
		DEVICE->CreateShaderResourceView(resource.Get(), srvDesc, mSrvHandle.CpuNext);
		mSrvHandle.CpuNext.ptr += DXGIMgr::I->GetCbvSrvUavDescriptorIncSize();
		mSrvHandle.GpuNext.ptr += DXGIMgr::I->GetCbvSrvUavDescriptorIncSize();
	}
}

void DescriptorHeap::CreateUnorderedAccessView(RComPtr<ID3D12Resource> resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc)
{
	// [resource]의 UAV를 [uavDesc]에 따라 생성한다.
	DEVICE->CreateUnorderedAccessView(resource.Get(), nullptr, uavDesc, mUavHandle.CpuNext);

	// UAV CPU/GPU의 Next를 다음 위치로 증가시킨다.
	mUavHandle.CpuNext.ptr += DXGIMgr::I->GetCbvSrvUavDescriptorIncSize();
	mUavHandle.GpuNext.ptr += DXGIMgr::I->GetCbvSrvUavDescriptorIncSize();
}

void DescriptorHeap::CreateDepthStencilView(RComPtr<ID3D12Resource> resource, const D3D12_DEPTH_STENCIL_VIEW_DESC* dsvDesc)
{
	DEVICE->CreateDepthStencilView(resource.Get(), dsvDesc, mDsvHandle.CpuNext);

	mDsvHandle.CpuNext.ptr += DXGIMgr::I->GetDsvDescriptorIncSize();
	mDsvHandle.GpuNext.ptr += DXGIMgr::I->GetDsvDescriptorIncSize();
}

void DescriptorHeap::Set()
{
	CMD_LIST->SetDescriptorHeaps(1, mCbvSrvUavHeap.GetAddressOf());
}