#include "stdafx.h"
#include "FrameResource.h"

#include "Light.h"
#pragma region MaterialData
MaterialData::MaterialData()
{
	MapIndices.fill(-1);
}
#pragma endregion

#pragma region FrameResource
FrameResource::FrameResource(ID3D12Device* pDevice, const std::array<int, BufferTypeCount>& bufferCounts)
{
	THROW_IF_FAILED(pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdAllocator.GetAddressOf())));

	PassCB = std::make_unique<UploadBuffer<PassConstants>>(pDevice, bufferCounts[static_cast<int>(BufferType::Pass)], true);
	PostPassCB = std::make_unique<UploadBuffer<PostPassConstants>>(pDevice, bufferCounts[static_cast<int>(BufferType::PostPass)], true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(pDevice, bufferCounts[static_cast<int>(BufferType::Object)], true);
	SkinMeshCB = std::make_unique<UploadBuffer<SkinnedConstants>>(pDevice, bufferCounts[static_cast<int>(BufferType::SkinMesh)], true);
	SsaoCB = std::make_unique<UploadBuffer<SsaoConstants>>(pDevice, bufferCounts[static_cast<int>(BufferType::Ssao)], true);
	MaterialBuffer = std::make_unique<UploadBuffer<MaterialData>>(pDevice, bufferCounts[static_cast<int>(BufferType::Material)], false);
}
#pragma endregion


#pragma region FrameResourceMgr
FrameResourceMgr::FrameResourceMgr(ID3D12Fence* fence)
	:
	mFence(fence),
	mFrameResourceCount(3)
{
	mBufferCounts[static_cast<int>(BufferType::Pass)]			= 2;
	mBufferCounts[static_cast<int>(BufferType::PostPass)]		= 1;
	mBufferCounts[static_cast<int>(BufferType::Object)]			= 2000;
	mBufferCounts[static_cast<int>(BufferType::SkinMesh)]		= 100;
	mBufferCounts[static_cast<int>(BufferType::Ssao)]			= 1;
	mBufferCounts[static_cast<int>(BufferType::Material)]		= 500;
	mBufferCounts[static_cast<int>(BufferType::ParticleSystem)] = 100;

	for (int bufferType = 0; bufferType < BufferTypeCount; ++bufferType) {
		for (int index = 0; index < mBufferCounts[bufferType]; ++index) {
			mAvailableIndices[bufferType].push(index);
		}
	}
}

const D3D12_GPU_VIRTUAL_ADDRESS FrameResourceMgr::GetPassCBGpuAddr(int elementIndex) const
{
	const auto& passCB = mCurrFrameResource->PassCB;
	return passCB->Resource()->GetGPUVirtualAddress() + elementIndex * passCB->GetElementByteSize();
}

const D3D12_GPU_VIRTUAL_ADDRESS FrameResourceMgr::GetObjCBGpuAddr(int elementIndex) const
{
	const auto& objectCB = mCurrFrameResource->ObjectCB;
	return objectCB->Resource()->GetGPUVirtualAddress() + elementIndex * objectCB->GetElementByteSize();
}

const D3D12_GPU_VIRTUAL_ADDRESS FrameResourceMgr::GetSKinMeshCBGpuAddr(int elementIndex) const
{
	const auto& skinMeshCB = mCurrFrameResource->SkinMeshCB;
	return skinMeshCB->Resource()->GetGPUVirtualAddress() + elementIndex * skinMeshCB->GetElementByteSize();
}

const D3D12_GPU_VIRTUAL_ADDRESS FrameResourceMgr::GetPostPassCBGpuAddr(int elementIndex) const
{
	const auto& postPassCB = mCurrFrameResource->PostPassCB;
	return postPassCB->Resource()->GetGPUVirtualAddress() + elementIndex * postPassCB->GetElementByteSize();
}

const D3D12_GPU_VIRTUAL_ADDRESS FrameResourceMgr::GetSSAOCBGpuAddr(int elementIndex) const
{
	const auto& ssaoCB = mCurrFrameResource->SsaoCB;
	return ssaoCB->Resource()->GetGPUVirtualAddress() + elementIndex * ssaoCB->GetElementByteSize();
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
		mFrameResources.push_back(std::make_unique<FrameResource>(pDevice, mBufferCounts));
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

void FrameResourceMgr::CopyData(const int elementIndex, const PassConstants& data)
{
	// �н� �� ��� ���۴� ���� �н� �� ���� �����ϸ� ���߿� �߰��� �� �ִ�.
	mCurrFrameResource->PassCB->CopyData(elementIndex, data);
}

void FrameResourceMgr::CopyData(const PostPassConstants& data)
{
	mCurrFrameResource->PostPassCB->CopyData(0, data);
}

void FrameResourceMgr::CopyData(const SsaoConstants& data)
{
	mCurrFrameResource->SsaoCB->CopyData(0, data);
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

void FrameResourceMgr::CopyData(int& elementIndex, const SkinnedConstants& data)
{
	constexpr UINT type = static_cast<UINT>(BufferType::SkinMesh);
	if (mActiveIndices[type].find(elementIndex) == mActiveIndices[type].end()) {

		if (mAvailableIndices[type].empty()) {
			return;
		}

		elementIndex = mAvailableIndices[type].front();
		mAvailableIndices[type].pop();
		mActiveIndices[type].insert(elementIndex);
	}

	mCurrFrameResource->SkinMeshCB->CopyData(elementIndex, data);
}

#pragma endregion