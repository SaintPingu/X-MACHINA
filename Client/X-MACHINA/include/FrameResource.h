#pragma once

#pragma region Include
#include "UploadBuffer.h"
#pragma endregion

#pragma region Struct
struct PassConstants {
    Matrix      MtxView{};
    Matrix      MtxProj{};
    Vec3        EyeW{};
    float       DeltaTime{};
    SceneLight  Lights{};
};

struct ObjectConstants {
    Matrix      MtxWorld{};
    Matrix      MtxSprite{};
};
#pragma endregion

#pragma region Class
struct FrameResource : private UnCopyable {
public:
    UINT64                              Fence{};
    ComPtr<ID3D12CommandAllocator>      CmdAllocator{};
    
    uptr<UploadBuffer<PassConstants>>   PassCB;       // �н� �� ��� ����
    uptr<UploadBuffer<ObjectConstants>> ObjectCB;     // ������Ʈ �� ��� ����

public:
#pragma region C/Dtor
    FrameResource(ID3D12Device* pDevice, UINT passCount, UINT objectCount);
    ~FrameResource() = default;
#pragma endregion
};

// ��� ������ ���ҽ��� �����ϴ� Ŭ����
class FrameResourceMgr {
private:
    int mFrameResourceCount{ 3 };
    int mPassCount{ 1 };
    int mObjectCount{ 1000 };

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
    const D3D12_GPU_VIRTUAL_ADDRESS GetPassCBGpuAddr() const;
    const D3D12_GPU_VIRTUAL_ADDRESS GetObjCBGpuAddr(int elementIndex) const;
#pragma endregion

    void CreateFrameResources(ID3D12Device* pDevice);
    void Update();

    // ��ü �Ҹ�� ������� �ʴ� ��� ���� �ε����� ��ȯ�ϴ� �Լ�
    void ReturnObjCBIdx(int elementIndex);

    // ������Ʈ �� ��� ���ۿ� ������ ����
    void CopyData(int& elementIndex, const ObjectConstants& data);

    // �н� �� ��� ���ۿ� ������ ����
    void CopyData(const PassConstants& data);
};

#pragma endregion

