#pragma once

#pragma region Include
#include "UploadBuffer.h"
#pragma endregion

#pragma region EnumClass
enum class BufferType : UINT {
    Pass = 0,
    Object,
    Material,
    _count
};

enum {
    BufferTypeCount = static_cast<UINT>(BufferType::_count)
};
#pragma endregion

#pragma region Struct
struct PassConstants {
public:
    Matrix      MtxView{};
    Matrix      MtxProj{};
    Vec3        EyeW{};
    float       DeltaTime{};
    SceneLight  Lights{};
    
    std::array<int, MRTCount> MRTTsIndices{};
    Vec3 Padding{};

public:
    PassConstants();
};

struct ObjectConstants {
    Matrix      MtxWorld{};
    Matrix      MtxSprite{};
    int         MatIndex{};
    Vec3        Padding{};
};

struct MaterialData {
    Vec4 Ambient{ Vector4::One() };
    Vec4 Diffuse{};
    Vec4 Specular{};
    Vec4 Emissive{};

    std::array<int, TextureMapCount> MapIndices;

public:
    MaterialData();
};
#pragma endregion

#pragma region Class
struct FrameResource : private UnCopyable {
public:
    UINT64                              Fence{};
    ComPtr<ID3D12CommandAllocator>      CmdAllocator{};
    
    uptr<UploadBuffer<PassConstants>>   PassCB{};       // �н� ��� ����
    uptr<UploadBuffer<ObjectConstants>> ObjectCB{};     // ������Ʈ ��� ����

    uptr<UploadBuffer<MaterialData>>    MaterialBuffer{};   // ��Ƽ���� ����

public:
#pragma region C/Dtor
    FrameResource(ID3D12Device* pDevice, int passCount, int objectCount, int materialCount);
    ~FrameResource() = default;
#pragma endregion
};

// ��� ������ ���ҽ��� �����ϴ� Ŭ����
class FrameResourceMgr {
private:
    int mFrameResourceCount;
    int mPassCount;
    int mObjectCount;
    int mMaterialCount;

    ID3D12Fence*                                mFence{};
    std::vector<uptr<FrameResource>>			mFrameResources{};
    FrameResource*                              mCurrFrameResource{};       // ���� ������ ���ҽ�
    int											mCurrFrameResourceIndex{};	// ���� ������ �ε���

    std::array<std::unordered_set<int>, BufferTypeCount>   mActiveIndices{};       // ������� �ε��� ����
    std::array<std::queue<int>, BufferTypeCount>           mAvailableIndices{};    // ��밡�� �ε��� ť

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
    const D3D12_GPU_VIRTUAL_ADDRESS GetMatBufferGpuAddr() const;
#pragma endregion
    void CreateFrameResources(ID3D12Device* pDevice);

    // ������ ���ҽ� �迭�� ��ȯ�ϸ� ��� ���� ������ ���ҽ��� ���´�.
    void Update();

    // ��ü �Ҹ�� ������� �ʴ� ���� �ε����� ��ȯ�ϴ� �Լ�
    void ReturnIndex(int elementIndex, BufferType bufferType);

    // �н� �� ��� ���ۿ� ������ ����
    void CopyData(const PassConstants& data);
    // ������Ʈ �� ��� ���ۿ� ������ ����
    void CopyData(int& elementIndex, const ObjectConstants& data);
    // ��Ƽ���� �� ��� ���ۿ� ������ ����
    void CopyData(int& elementIndex, const MaterialData& data);
};

#pragma endregion

