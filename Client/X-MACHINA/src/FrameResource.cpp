#include "stdafx.h"
#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* pDevice, UINT passCount, UINT objectCount)
{
	THROW_IF_FAILED(pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdAllocator.GetAddressOf())));

	PassCB = std::make_unique<UploadBuffer<PassConstants>>(pDevice, passCount, true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(pDevice, objectCount, true);
}



FrameResourceMgr::FrameResourceMgr(ID3D12Fence* fence)
	:
	mFence(fence)
{
}

const D3D12_GPU_VIRTUAL_ADDRESS FrameResourceMgr::GetPassCBGpuAddr() const
{
	const auto& passCB = mCurrFrameResource->PassCB;
	return passCB->Resource()->GetGPUVirtualAddress();
}

const D3D12_GPU_VIRTUAL_ADDRESS FrameResourceMgr::GetObjCBGpuAddr(int elementIndex) const
{
	const auto& objectCB = mCurrFrameResource->ObjectCB;
	return objectCB->Resource()->GetGPUVirtualAddress() + elementIndex * objectCB->GetElementByteSize();
}

void FrameResourceMgr::CreateFrameResources(ID3D12Device* pDevice)
{
	// 현재 사용중이지 않은 버퍼의 인덱스를 담은 큐를 최대 개수만큼 초기화한다. (현재 : 0~999)
	for (int i = 0; i < mObjectCount; ++i) {
		mAvailableObjCBIdxes.push(i);
	}

	// 프레임 리소스 최대 개수만큼 프레임 리소스를 생성한다.
	for (int i = 0; i < mFrameResourceCount; ++i) {
		mFrameResources.push_back(std::make_unique<FrameResource>(pDevice, mPassCount, mObjectCount));
	}

	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();
}

void FrameResourceMgr::Update()
{
	// 프레임마다 프레임 리소스를 순환하여 현재의 프레임 리소스를 저장한다.
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % mFrameResourceCount;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

	// 프레임 리소스 개수만큼의 순환이 끝났음에도 GPU가 첫 프레임의 리소스를 처리하지 않았다면 동기화 해야 한다.
	if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
		THROW_IF_FAILED(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void FrameResourceMgr::ReturnObjCBIdx(int elementIndex)
{
	if (mActiveObjCBIdxes.erase(elementIndex)) {
		mAvailableObjCBIdxes.push(elementIndex);
	}
}

void FrameResourceMgr::CopyData(int& elementIndex, const ObjectConstants& data)
{
	// 사용중인 인덱스 집합에서 elementIndex를 찾지 못한 경우
	if (mActiveObjCBIdxes.find(elementIndex) == mActiveObjCBIdxes.end()) {

		// 사용 가능한 인덱스가 없을 경우 메모리를 복사하지 않는다.
		if (mAvailableObjCBIdxes.empty()) {
			return;
		}

		// 사용 가능한 인덱스 큐에서 맨 앞의 인덱스를 가져오고 삭제한다.
		elementIndex = mAvailableObjCBIdxes.front();
		mAvailableObjCBIdxes.pop();
		mActiveObjCBIdxes.insert(elementIndex);
	}

	// 매핑된 메모리에 데이터 복사
	mCurrFrameResource->ObjectCB->CopyData(elementIndex, data);
}

void FrameResourceMgr::CopyData(const PassConstants& data)
{
	mCurrFrameResource->PassCB->CopyData(0, data);
}

