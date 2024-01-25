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
    
    uptr<UploadBuffer<PassConstants>>   PassCB{};       // 패스 상수 버퍼
    uptr<UploadBuffer<ObjectConstants>> ObjectCB{};     // 오브젝트 상수 버퍼

    uptr<UploadBuffer<MaterialData>>    MaterialBuffer{};   // 머티리얼 버퍼

public:
#pragma region C/Dtor
    FrameResource(ID3D12Device* pDevice, int passCount, int objectCount, int materialCount);
    ~FrameResource() = default;
#pragma endregion
};

// 모든 프레임 리소스를 관리하는 클래스
class FrameResourceMgr {
private:
    int mFrameResourceCount;
    int mPassCount;
    int mObjectCount;
    int mMaterialCount;

    ID3D12Fence*                                mFence{};
    std::vector<uptr<FrameResource>>			mFrameResources{};
    FrameResource*                              mCurrFrameResource{};       // 현재 프레임 리소스
    int											mCurrFrameResourceIndex{};	// 현재 프레임 인덱스

    std::array<std::unordered_set<int>, BufferTypeCount>   mActiveIndices{};       // 사용중인 인덱스 집합
    std::array<std::queue<int>, BufferTypeCount>           mAvailableIndices{};    // 사용가능 인덱스 큐

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

    // 프레임 리소스 배열을 순환하며 사용 가능 프레임 리소스를 얻어온다.
    void Update();

    // 객체 소멸시 사용하지 않는 버퍼 인덱스를 반환하는 함수
    void ReturnIndex(int elementIndex, BufferType bufferType);

    // 패스 당 상수 버퍼에 데이터 복사
    void CopyData(const PassConstants& data);
    // 오브젝트 당 상수 버퍼에 데이터 복사
    void CopyData(int& elementIndex, const ObjectConstants& data);
    // 머티리얼 당 상수 버퍼에 데이터 복사
    void CopyData(int& elementIndex, const MaterialData& data);
};

#pragma endregion

