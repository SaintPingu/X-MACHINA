#pragma once

#pragma region Include
#include "UploadBuffer.h"
#pragma endregion

#pragma region Struct
struct ObjectConstants {
	Matrix MtxWorld{};
};
#pragma endregion

#pragma region Class
struct FrameResource : private UnCopyable {
public:
    UINT64                              Fence{};
    ComPtr<ID3D12CommandAllocator>      CmdAllocator{};

    uptr<UploadBuffer<ObjectConstants>> ObjectCB{};     // 오브젝트 당 상수 버퍼

public:
    FrameResource(ID3D12Device* pDevice, UINT objectCount);
    ~FrameResource() = default;
};
#pragma endregion

