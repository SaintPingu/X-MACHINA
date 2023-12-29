#include "stdafx.h"
#include "DDSTextureLoader12.h"
#include "DXGIMgr.h"

UINT gCbvSrvDescriptorIncSize = 0;
UINT gRtvDescriptorIncSize = 0;

//////////////////* DirectX *//////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace D3DUtil {
	void CreateBufferResource(const void* data, UINT byteSize, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceStates, ComPtr<ID3D12Resource>& uploadBuffer, ComPtr<ID3D12Resource>& buffer)
	{
		D3D12_HEAP_PROPERTIES heapPropertiesDesc{};
		heapPropertiesDesc.Type = heapType;
		heapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapPropertiesDesc.CreationNodeMask = 1;
		heapPropertiesDesc.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = byteSize;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_RESOURCE_STATES resourceInitialStates = D3D12_RESOURCE_STATE_COMMON;
		switch (heapType) {
		case D3D12_HEAP_TYPE_UPLOAD:
			resourceInitialStates = D3D12_RESOURCE_STATE_GENERIC_READ;
			break;
		case D3D12_HEAP_TYPE_READBACK:
			resourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;
			break;
		default:
			break;
		}

		HRESULT hResult = device->CreateCommittedResource(&heapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc, resourceInitialStates, nullptr, IID_PPV_ARGS(&buffer));
		assert(SUCCEEDED(hResult));

		if (data)
		{
			switch (heapType)
			{
			case D3D12_HEAP_TYPE_DEFAULT:
			{
				heapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
				hResult = device->CreateCommittedResource(&heapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
				assert(SUCCEEDED(hResult));

				if (uploadBuffer) {
					D3D12_RANGE readRange = { 0, 0 };
					UINT8* bufferDataBegin{};
					uploadBuffer->Map(0, &readRange, (void**)&bufferDataBegin);
					memcpy(bufferDataBegin, data, byteSize);
					uploadBuffer->Unmap(0, nullptr);

					cmdList->CopyResource(buffer.Get(), uploadBuffer.Get());
				}

				D3DUtil::ResourceTransition(buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, resourceStates);

				break;
			}
			case D3D12_HEAP_TYPE_UPLOAD:
			{
				D3D12_RANGE readRange = { 0, 0 };
				UINT8* bufferDataBegin = nullptr;
				buffer->Map(0, &readRange, (void**)&bufferDataBegin);
				memcpy(bufferDataBegin, data, byteSize);
				buffer->Unmap(0, nullptr);
				break;
			}
			case D3D12_HEAP_TYPE_READBACK:
				break;
			}
		}
	}




	void CreateVertexBufferViews(std::vector<D3D12_VERTEX_BUFFER_VIEW>& vertexBufferViews, size_t vertexCount, const BufferViews& bufferViews)
	{
		D3D12_VERTEX_BUFFER_VIEW view{};

		const size_t vec3Size = sizeof(Vec3) * vertexCount;
		const size_t vec2Size = sizeof(Vec2) * vertexCount;

		if (bufferViews.vertexBuffer) {
			view.BufferLocation = bufferViews.vertexBuffer->GetGPUVirtualAddress();
			view.StrideInBytes = sizeof(Vec3);
			view.SizeInBytes = vec3Size;
			vertexBufferViews.emplace_back(view);
		}

		if (bufferViews.normalBuffer) {
			view.BufferLocation = bufferViews.normalBuffer->GetGPUVirtualAddress();
			view.StrideInBytes = sizeof(Vec3);
			view.SizeInBytes = vec3Size;
			vertexBufferViews.emplace_back(view);
		}

		if (bufferViews.UV0Buffer) {
			view.BufferLocation = bufferViews.UV0Buffer->GetGPUVirtualAddress();
			view.StrideInBytes = sizeof(Vec2);
			view.SizeInBytes = vec2Size;
			vertexBufferViews.emplace_back(view);
		}

		if (bufferViews.UV1Buffer) {
			view.BufferLocation = bufferViews.UV1Buffer->GetGPUVirtualAddress();
			view.StrideInBytes = sizeof(Vec2);
			view.SizeInBytes = vec2Size;
			vertexBufferViews.emplace_back(view);
		}

		if (bufferViews.tangentBuffer) {
			view.BufferLocation = bufferViews.tangentBuffer->GetGPUVirtualAddress();
			view.StrideInBytes = sizeof(Vec3);
			view.SizeInBytes = vec3Size;
			vertexBufferViews.emplace_back(view);
		}

		if (bufferViews.biTangentBuffer) {
			view.BufferLocation = bufferViews.biTangentBuffer->GetGPUVirtualAddress();
			view.StrideInBytes = sizeof(Vec3);
			view.SizeInBytes = vec3Size;
			vertexBufferViews.emplace_back(view);
		}
	}




	void CreateTextureResourceFromDDSFile(std::wstring fileName, ComPtr<ID3D12Resource>& uploadBuffer, ComPtr<ID3D12Resource>& texture, D3D12_RESOURCE_STATES resourceStates)
	{
		uptr<uint8_t[]> ddsData;
		std::vector<D3D12_SUBRESOURCE_DATA> vSubresources;
		DDS_ALPHA_MODE ddsAlphaMode = DDS_ALPHA_MODE_UNKNOWN;
		bool bIsCubeMap{ false };

		HRESULT hResult = DirectX::LoadDDSTextureFromFileEx(device.Get(), fileName.c_str(), 0, D3D12_RESOURCE_FLAG_NONE, DDS_LOADER_DEFAULT, &texture, ddsData, vSubresources, &ddsAlphaMode, &bIsCubeMap);
		assert(SUCCEEDED(hResult));

		D3D12_HEAP_PROPERTIES heapPropertiesDesc{};
		heapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapPropertiesDesc.CreationNodeMask = 1;
		heapPropertiesDesc.VisibleNodeMask = 1;

		UINT nSubResources = (UINT)vSubresources.size();
		UINT64 nBytes = GetRequiredIntermediateSize(texture.Get(), 0, nSubResources);

		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //Upload Heap에는 텍스쳐를 생성할 수 없음
		resourceDesc.Alignment = 0;
		resourceDesc.Width = nBytes;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		hResult = device->CreateCommittedResource(&heapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
		assert(SUCCEEDED(hResult));

		//리소스 데이터를 Upload Heap에 복사하고 이를 texture 리소스에 복사
		::UpdateSubresources(cmdList.Get(), texture.Get(), uploadBuffer.Get(), 0, 0, nSubResources, &vSubresources[0]);

		D3DUtil::ResourceTransition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, resourceStates);
	}

	ComPtr<ID3D12Resource> CreateTexture2DResource(UINT width, UINT height, UINT elements, UINT mipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS resourceFlags, D3D12_RESOURCE_STATES resourceStates, D3D12_CLEAR_VALUE* clearValue)
	{
		ComPtr<ID3D12Resource> texture{};

		D3D12_HEAP_PROPERTIES heapPropertiesDesc{};
		heapPropertiesDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapPropertiesDesc.CreationNodeMask = 1;
		heapPropertiesDesc.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC textureResourceDesc{};
		textureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureResourceDesc.Alignment = 0;
		textureResourceDesc.Width = width;
		textureResourceDesc.Height = height;
		textureResourceDesc.DepthOrArraySize = elements;
		textureResourceDesc.MipLevels = mipLevels;
		textureResourceDesc.Format = dxgiFormat;
		textureResourceDesc.SampleDesc.Count = 1;
		textureResourceDesc.SampleDesc.Quality = 0;
		textureResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		textureResourceDesc.Flags = resourceFlags;

		HRESULT hResult = device->CreateCommittedResource(&heapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &textureResourceDesc, resourceStates, clearValue, IID_PPV_ARGS(&texture));
		assert(SUCCEEDED(hResult));

		return texture;
	}

	void ResourceTransition(RComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
	{
		D3D12_RESOURCE_BARRIER resourceBarrier{};
		resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceBarrier.Transition.pResource = resource.Get();
		resourceBarrier.Transition.StateBefore = stateBefore;
		resourceBarrier.Transition.StateAfter = stateAfter;
		resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		cmdList->ResourceBarrier(1, &resourceBarrier);
	}
}



//////////////////* Math *//////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Math {
	int RoundToNearestMultiple(int value, int multiple)
	{
		const int remainder = value % multiple;
		int result{};

		if (remainder <= multiple / 2) {
			return value - remainder;
		}

		return value + (multiple - remainder);
	}
}




//////////////////* Others *//////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrintMessage(const char* message)
{
	MessageBoxA(nullptr, message, "Message", MB_OK | MB_ICONERROR);
}


void PrintErrorBlob(RComPtr<ID3DBlob> errorBlob)
{
#if defined(_DEBUG)
	if (errorBlob) {
		PrintMessage(static_cast<const char*>(errorBlob->GetBufferPointer()));
	}
#endif
}