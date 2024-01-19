#pragma once

#pragma region ClassForwardDecl
class Transform;
#pragma endregion

// 여러개의 상수 버퍼를 한 번에 관리하기 위한 클래스
// 복사가 되지 않아야 한다.
template<typename T>
class UploadBuffer  {
private:
    ComPtr<ID3D12Resource>  mUploadBuffer{};
    BYTE* mMappedData{};

    int     mElementCount{};
    size_t  mElementByteSize{};
    bool    mIsConstantBuffer{};

public:
    // mElementByteSize * mElementCount 만큼의 버퍼를 사용한다.
    UploadBuffer(ID3D12Device* pDevice, int elementCount, bool isConstantBuffer)
        :
        mIsConstantBuffer(isConstantBuffer),
        mElementCount(elementCount),
        mElementByteSize(sizeof(T))
    {
        // 상수 버퍼로 사용할 경우 256의 배수가 되도록 한다.
        if (isConstantBuffer)
            mElementByteSize = D3DUtil::CalcConstantBuffSize(sizeof(T));

        D3D12_HEAP_PROPERTIES heapProperties{};
        heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProperties.CreationNodeMask = 1;
        heapProperties.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC resourceDesc{};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Alignment = 0;
        resourceDesc.Width = static_cast<UINT64>(mElementByteSize) * mElementCount;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        THROW_IF_FAILED(pDevice->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&mUploadBuffer)));

        THROW_IF_FAILED(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData)));
    }

    // 소멸자 호출시 업로드 버퍼 매핑 해제
    ~UploadBuffer() {
        if (mUploadBuffer != nullptr)
            mUploadBuffer->Unmap(0, nullptr);

        mMappedData = nullptr;
    }

public:
    const size_t GetElementByteSize() const {
        return mElementByteSize;
    }

    ID3D12Resource* Resource()const {
        return mUploadBuffer.Get();
    }

    // 매핑된 메모리에 데이터를 복사하는 함수
    void CopyData(int elementIndex, const T& data) {
        memcpy(&mMappedData[elementIndex * mElementByteSize], &data, sizeof(T));
    }
};



