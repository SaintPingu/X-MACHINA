#include "stdafx.h"
#include "FrameResource.h"

#pragma region MaterialData
MaterialData::MaterialData()
{
	MapIndices.fill(-1);
}
#pragma endregion

#pragma region FrameResource
FrameResource::FrameResource(ID3D12Device* pDevice, int passCount, int postPassCount, int objectCount, int materialCount)
{
	THROW_IF_FAILED(pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdAllocator.GetAddressOf())));

	PassCB = std::make_unique<UploadBuffer<PassConstants>>(pDevice, passCount, true);
	PostPassCB = std::make_unique<UploadBuffer<PostPassConstants>>(pDevice, postPassCount, true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(pDevice, objectCount, true);
	MaterialBuffer = std::make_unique<UploadBuffer<MaterialData>>(pDevice, materialCount, false);
}
#pragma endregion


#pragma region FrameResourceMgr
FrameResourceMgr::FrameResourceMgr(ID3D12Fence* fence)
	:
	mFence(fence),
	mFrameResourceCount(3),
	mPassCount(1),
	mPostPassCount(1),
	mObjectCount(2000),
	mMaterialCount(500)
{
	mActiveIndices[static_cast<int>(BufferType::Pass)].reserve(mPassCount);
	for (int i = 0; i < mPassCount; ++i) {
		mAvailableIndices[static_cast<int>(BufferType::Pass)].push(i);
	}

	mActiveIndices[static_cast<int>(BufferType::Pass)].reserve(mPassCount);
	for (int i = 0; i < mPostPassCount; ++i) {
		mAvailableIndices[static_cast<int>(BufferType::PostPass)].push(i);
	}

	mActiveIndices[static_cast<int>(BufferType::Object)].reserve(mObjectCount);
	for (int i = 0; i < mObjectCount; ++i) {
		mAvailableIndices[static_cast<int>(BufferType::Object)].push(i);
	}

	mActiveIndices[static_cast<int>(BufferType::Material)].reserve(mMaterialCount);
	for (int i = 0; i < mMaterialCount; ++i) {
		mAvailableIndices[static_cast<int>(BufferType::Material)].push(i);
	}
}

const D3D12_GPU_VIRTUAL_ADDRESS FrameResourceMgr::GetPassCBGpuAddr(int elementIndex) const
{
	const auto& passCB = mCurrFrameResource->PassCB;
	return passCB->Resource()->GetGPUVirtualAddress() + elementIndex * passCB->GetElementByteSize();
}

const D3D12_GPU_VIRTUAL_ADDRESS FrameResourceMgr::GetPostPassCBGpuAddr(int elementIndex) const
{
	const auto& postPassCB = mCurrFrameResource->PostPassCB;
	return postPassCB->Resource()->GetGPUVirtualAddress() + elementIndex * postPassCB->GetElementByteSize();
}

const D3D12_GPU_VIRTUAL_ADDRESS FrameResourceMgr::GetObjCBGpuAddr(int elementIndex) const
{
	const auto& objectCB = mCurrFrameResource->ObjectCB;
	return objectCB->Resource()->GetGPUVirtualAddress() + elementIndex * objectCB->GetElementByteSize();
}

const D3D12_GPU_VIRTUAL_ADDRESS FrameResourceMgr::GetMatBufferGpuAddr(int elementIndex) const
{
	const auto& materialBuffer = mCurrFrameResource->MaterialBuffer;
	return materialBuffer->Resource()->GetGPUVirtualAddress() + elementIndex * materialBuffer->GetElementByteSize();
}

void FrameResourceMgr::CreateFrameResources(ID3D12Device* pDevice)
{
	// ������ ���ҽ� �ִ� ������ŭ ������ ���ҽ��� �����Ѵ�.
	for (int i = 0; i < mFrameResourceCount; ++i) {
		mFrameResources.push_back(std::make_unique<FrameResource>(pDevice, mPassCount, mPostPassCount, mObjectCount, mMaterialCount));
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
		HANDLE eventHandle = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		THROW_IF_FAILED(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void FrameResourceMgr::ReturnIndex(int elementIndex, BufferType bufferType)
{
	const UINT& type = static_cast<UINT>(bufferType);

	// ��� ������Ʈ �ε����� -1�� �ʱ�ȭ�Ͽ���.
	if (elementIndex != -1) {
		if (mActiveIndices[type].erase(elementIndex)) {
			mAvailableIndices[type].push(elementIndex);
		}
	}
}

void FrameResourceMgr::CopyData(const PassConstants& data)
{
	// �н� �� ��� ���۴� ���� �н� �� ���� �����ϸ� ���߿� �߰��� �� �ִ�.
	mCurrFrameResource->PassCB->CopyData(0, data);
}

void FrameResourceMgr::CopyData(const PostPassConstants& data)
{
	mCurrFrameResource->PostPassCB->CopyData(0, data);
}

void FrameResourceMgr::CopyData(int& elementIndex, const ObjectConstants& data)
{
	constexpr UINT type = static_cast<UINT>(BufferType::Object);
	// ������� �ε��� ���տ��� elementIndex�� ã�� ���� ���
	if (mActiveIndices[type].find(elementIndex) == mActiveIndices[type].end()) {

		// ��� ������ �ε����� ���� ��� �޸𸮸� �������� �ʴ´�.
		if (mAvailableIndices[type].empty()) {
			return;
		}

		// ��� ������ �ε��� ť���� �� ���� �ε����� �������� �����Ѵ�.
		elementIndex = mAvailableIndices[type].front();
		mAvailableIndices[type].pop();
		mActiveIndices[type].insert(elementIndex);
	}

	// ���ε� �޸𸮿� ������ ����
	mCurrFrameResource->ObjectCB->CopyData(elementIndex, data);
}

void FrameResourceMgr::CopyData(int& elementIndex, const MaterialData& data)
{
	constexpr UINT type = static_cast<UINT>(BufferType::Material);
	if (mActiveIndices[type].find(elementIndex) == mActiveIndices[type].end()) {

		if (mAvailableIndices[type].empty()) {
			return;
		}

		elementIndex = mAvailableIndices[type].front();
		mAvailableIndices[type].pop();
		mActiveIndices[type].insert(elementIndex);
	}

	mCurrFrameResource->MaterialBuffer->CopyData(elementIndex, data);
}

#pragma endregion