#include "stdafx.h"
#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* pDevice, UINT objectCount)
{
	THROW_IF_FAILED(pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdAllocator.GetAddressOf())));

	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(pDevice, objectCount, true);
}



FrameResourceMgr::FrameResourceMgr(ID3D12Fence* fence)
	:
	mFence(fence)
{
}

void FrameResourceMgr::CreateFrameResources(ID3D12Device* pDevice)
{
	// ���� ��������� ���� ������ �ε����� ���� ť�� �ִ� ������ŭ �ʱ�ȭ�Ѵ�. (���� : 0~999)
	for (int i = 0; i < mMaxObjectCount; ++i) {
		mAvailableObjCBIdxes.push(i);
	}

	// ������ ���ҽ� �ִ� ������ŭ ������ ���ҽ��� �����Ѵ�.
	for (int i = 0; i < mFrameResourceCount; ++i) {
		mFrameResources.push_back(std::make_unique<FrameResource>(pDevice, mMaxObjectCount));
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

