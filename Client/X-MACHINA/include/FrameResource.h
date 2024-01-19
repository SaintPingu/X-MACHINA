#pragma once

#pragma region Include
#include "UploadBuffer.h"
#pragma endregion

#pragma region Class
struct FrameResource : private UnCopyable {
public:
    UINT64                              Fence{};
    ComPtr<ID3D12CommandAllocator>      CmdAllocator{};
    
    uptr<UploadBuffer<ObjectConstants>> ObjectCB;     // ������Ʈ �� ��� ����

public:
#pragma region C/Dtor
    FrameResource(ID3D12Device* pDevice, UINT objectCount);
    ~FrameResource() = default;
#pragma endregion
};

// ��� ������ ���ҽ��� �����ϴ� Ŭ����
class FrameResourceMgr {
private:
    static constexpr UINT mFrameResourceCount   = 1;
    static constexpr UINT mMaxObjectCount       = 1000;

    ID3D12Fence*                                mFence{};
    std::vector<uptr<FrameResource>>			mFrameResources{};
    FrameResource*                              mCurrFrameResource{};       // ���� ������ ���ҽ�
    int											mCurrFrameResourceIndex{};	// ���� ������ �ε���

    std::unordered_set<int>                     mActiveObjCBIdxes{};
    std::queue<int>                             mAvailableObjCBIdxes{};

public:
#pragma region C/Dtor
    FrameResourceMgr(ID3D12Fence* fence);
    ~FrameResourceMgr() = default;
#pragma endregion

public:
#pragma region Getter
    FrameResource* GetCurrFrameResource() const { return mCurrFrameResource; }
    const D3D12_GPU_VIRTUAL_ADDRESS GetObjCBGpuAddr(int elementIndex) const {  
        const auto& objectCB = mCurrFrameResource->ObjectCB;
        return objectCB->Resource()->GetGPUVirtualAddress() + elementIndex * objectCB->GetElementByteSize();
    }
#pragma endregion

    void CreateFrameResources(ID3D12Device* pDevice);
    void Update();

    // ��ü �Ҹ�� ������� �ʴ� ��� ���� �ε����� ��ȯ�ϴ� �Լ�
    void ReturnObjCBIdx(int elementIndex);

    // ������Ʈ �� ��� ���ۿ� ������ ����
    void CopyData(int& elementIndex, const ObjectConstants& data);
};

#pragma endregion

