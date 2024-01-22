#include "stdafx.h"
#include "FrameResource.h"

#pragma region FrameResource
FrameResource::FrameResource(ID3D12Device* pDevice, int passCount, int objectCount, int materialCount)
{
	THROW_IF_FAILED(pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdAllocator.GetAddressOf())));

	PassCB = std::make_unique<UploadBuffer<PassConstants>>(pDevice, passCount, true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(pDevice, objectCount, true);
	MaterialSB = std::make_unique<UploadBuffer<MaterialData>>(pDevice, materialCount, false);
}
#pragma endregion


#pragma region FrameResourceMgr
FrameResourceMgr::FrameResourceMgr(ID3D12Fence* fence)
	:
	mFence(fence),
	mFrameResourceCount(3),
	mPassCount(1),
	mObjectCount(1000),
	mMaterialCount(500)
{
}

const D3D12_GPU_VIRTUAL_ADDRESS FrameResourceMgr::GetPassCBGpuAddr() const
{
	const auto& passCB = mCurrFrameResource->PassCB;
	return passCB->Resource()->GetGPUVirtualAddress();
}

const D3D12_GPU_VIRTUAL_ADDRESS FrameResourceMgr::GetMatSBGpuAddr() const
{
	const auto& materialSB = mCurrFrameResource->MaterialSB;
	return materialSB->Resource()->GetGPUVirtualAddress();
}

const D3D12_GPU_VIRTUAL_ADDRESS FrameResourceMgr::GetObjCBGpuAddr(int elementIndex) const
{
	const auto& objectCB = mCurrFrameResource->ObjectCB;
	return objectCB->Resource()->GetGPUVirtualAddress() + elementIndex * objectCB->GetElementByteSize();
}

void FrameResourceMgr::CreateFrameResources(ID3D12Device* pDevice)
{
	// 사용 가능한 버퍼 인덱스를 설정
	for (int i = 0; i < mObjectCount; ++i) {
		mAvailableObjCBIdxes.push(i);
	}
	for (int i = 0; i < mMaterialCount; ++i) {
		mAvailableMaterialSBIdxes.push(i);
	}

	// 프레임 리소스 최대 개수만큼 프레임 리소스를 생성한다.
	for (int i = 0; i < mFrameResourceCount; ++i) {
		mFrameResources.push_back(std::make_unique<FrameResource>(pDevice, mPassCount, mObjectCount, mMaterialCount));
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

void FrameResourceMgr::CopyData(const PassConstants& data)
{
	mCurrFrameResource->PassCB->CopyData(0, data);
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

void FrameResourceMgr::CopyData(int& elementIndex, const MaterialData& data)
{
	if (mActiveMaterialSBIdxes.find(elementIndex) == mActiveMaterialSBIdxes.end()) {

		if (mAvailableMaterialSBIdxes.empty()) {
			return;
		}

		elementIndex = mAvailableMaterialSBIdxes.front();
		mAvailableMaterialSBIdxes.pop();
		mActiveMaterialSBIdxes.insert(elementIndex);
	}

	mCurrFrameResource->MaterialSB->CopyData(elementIndex, data);
}

#pragma endregion

