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
	// ��� ������ ���� �ε����� ����
	for (int i = 0; i < mObjectCount; ++i) {
		mAvailableObjCBIdxes.push(i);
	}
	for (int i = 0; i < mMaterialCount; ++i) {
		mAvailableMaterialSBIdxes.push(i);
	}

	// ������ ���ҽ� �ִ� ������ŭ ������ ���ҽ��� �����Ѵ�.
	for (int i = 0; i < mFrameResourceCount; ++i) {
		mFrameResources.push_back(std::make_unique<FrameResource>(pDevice, mPassCount, mObjectCount, mMaterialCount));
	}

	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();
}

void FrameResourceMgr::Update()
{
	// �����Ӹ��� ������ ���ҽ��� ��ȯ�Ͽ� ������ ������ ���ҽ��� �����Ѵ�.
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % mFrameResourceCount;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

	// ������ ���ҽ� ������ŭ�� ��ȯ�� ���������� GPU�� ù �������� ���ҽ��� ó������ �ʾҴٸ� ����ȭ �ؾ� �Ѵ�.
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
	// ������� �ε��� ���տ��� elementIndex�� ã�� ���� ���
	if (mActiveObjCBIdxes.find(elementIndex) == mActiveObjCBIdxes.end()) {

		// ��� ������ �ε����� ���� ��� �޸𸮸� �������� �ʴ´�.
		if (mAvailableObjCBIdxes.empty()) {
			return;
		}

		// ��� ������ �ε��� ť���� �� ���� �ε����� �������� �����Ѵ�.
		elementIndex = mAvailableObjCBIdxes.front();
		mAvailableObjCBIdxes.pop();
		mActiveObjCBIdxes.insert(elementIndex);
	}

	// ���ε� �޸𸮿� ������ ����
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

