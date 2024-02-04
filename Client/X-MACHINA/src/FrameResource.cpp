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
	// 프레임 리소스 최대 개수만큼 프레임 리소스를 생성한다.
	for (int i = 0; i < mFrameResourceCount; ++i) {
		mFrameResources.push_back(std::make_unique<FrameResource>(pDevice, mPassCount, mPostPassCount, mObjectCount, mMaterialCount));
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
		HANDLE eventHandle = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		THROW_IF_FAILED(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void FrameResourceMgr::ReturnIndex(int elementIndex, BufferType bufferType)
{
	const UINT& type = static_cast<UINT>(bufferType);

	// 모든 오브젝트 인덱스를 -1로 초기화하였다.
	if (elementIndex != -1) {
		if (mActiveIndices[type].erase(elementIndex)) {
			mAvailableIndices[type].push(elementIndex);
		}
	}
}

void FrameResourceMgr::CopyData(const PassConstants& data)
{
	// 패스 당 상수 버퍼는 메인 패스 한 개만 존재하며 나중에 추가될 수 있다.
	mCurrFrameResource->PassCB->CopyData(0, data);
}

void FrameResourceMgr::CopyData(const PostPassConstants& data)
{
	mCurrFrameResource->PostPassCB->CopyData(0, data);
}

void FrameResourceMgr::CopyData(int& elementIndex, const ObjectConstants& data)
{
	constexpr UINT type = static_cast<UINT>(BufferType::Object);
	// 사용중인 인덱스 집합에서 elementIndex를 찾지 못한 경우
	if (mActiveIndices[type].find(elementIndex) == mActiveIndices[type].end()) {

		// 사용 가능한 인덱스가 없을 경우 메모리를 복사하지 않는다.
		if (mAvailableIndices[type].empty()) {
			return;
		}

		// 사용 가능한 인덱스 큐에서 맨 앞의 인덱스를 가져오고 삭제한다.
		elementIndex = mAvailableIndices[type].front();
		mAvailableIndices[type].pop();
		mActiveIndices[type].insert(elementIndex);
	}

	// 매핑된 메모리에 데이터 복사
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