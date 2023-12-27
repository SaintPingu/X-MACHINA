#include "stdafx.h"
#include "DDSTextureLoader12.h"
#include "DXGIMgr.h"

UINT gnCbvSrvDescriptorIncrementSize = 0;
UINT gnRtvDescriptorIncrementSize = 0;

//////////////////* DirectX *//////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
	if (heapType == D3D12_HEAP_TYPE_UPLOAD) resourceInitialStates = D3D12_RESOURCE_STATE_GENERIC_READ;
	else if (heapType == D3D12_HEAP_TYPE_READBACK) resourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;

	HRESULT hResult = device->CreateCommittedResource(&heapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc, resourceInitialStates, NULL, IID_PPV_ARGS(&buffer));
	assert(SUCCEEDED(hResult));

	if (data)
	{
		switch (heapType)
		{
		case D3D12_HEAP_TYPE_DEFAULT:
		{
			heapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
			device->CreateCommittedResource(&heapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&uploadBuffer));
			assert(SUCCEEDED(hResult));

			if (uploadBuffer) {
				D3D12_RANGE readRange = { 0, 0 };
				UINT8* bufferDataBegin = NULL;
				uploadBuffer->Map(0, &readRange, (void**)&bufferDataBegin);
				memcpy(bufferDataBegin, data, byteSize);
				uploadBuffer->Unmap(0, NULL);

				cmdList->CopyResource(buffer.Get(), uploadBuffer.Get());
			}

			SynchronizeResourceTransition(buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, resourceStates);

			break;
		}
		case D3D12_HEAP_TYPE_UPLOAD:
		{
			D3D12_RANGE readRange = { 0, 0 };
			UINT8* bufferDataBegin = NULL;
			buffer->Map(0, &readRange, (void**)&bufferDataBegin);
			memcpy(bufferDataBegin, data, byteSize);
			buffer->Unmap(0, NULL);
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

	if (bufferViews.vertexBuffer) {
		view.BufferLocation = bufferViews.vertexBuffer->GetGPUVirtualAddress();
		view.StrideInBytes = sizeof(Vec3);
		view.SizeInBytes = sizeof(Vec3) * vertexCount;
		vertexBufferViews.emplace_back(view);
	}

	if (bufferViews.normalBuffer) {
		view.BufferLocation = bufferViews.normalBuffer->GetGPUVirtualAddress();
		view.StrideInBytes = sizeof(Vec3);
		view.SizeInBytes = sizeof(Vec3) * vertexCount;
		vertexBufferViews.emplace_back(view);
	}

	if (bufferViews.UV0Buffer) {
		view.BufferLocation = bufferViews.UV0Buffer->GetGPUVirtualAddress();
		view.StrideInBytes = sizeof(Vec2);
		view.SizeInBytes = sizeof(Vec2) * vertexCount;
		vertexBufferViews.emplace_back(view);
	}

	if (bufferViews.UV1Buffer) {
		view.BufferLocation = bufferViews.UV1Buffer->GetGPUVirtualAddress();
		view.StrideInBytes = sizeof(Vec2);
		view.SizeInBytes = sizeof(Vec2) * vertexCount;
		vertexBufferViews.emplace_back(view);
	}

	if (bufferViews.tangentBuffer) {
		view.BufferLocation = bufferViews.tangentBuffer->GetGPUVirtualAddress();
		view.StrideInBytes = sizeof(Vec3);
		view.SizeInBytes = sizeof(Vec3) * vertexCount;
		vertexBufferViews.emplace_back(view);
	}

	if (bufferViews.biTangentBuffer) {
		view.BufferLocation = bufferViews.biTangentBuffer->GetGPUVirtualAddress();
		view.StrideInBytes = sizeof(Vec3);
		view.SizeInBytes = sizeof(Vec3) * vertexCount;
		vertexBufferViews.emplace_back(view);
	}
}




void CreateTextureResourceFromDDSFile(std::wstring fileName, ComPtr<ID3D12Resource>& uploadBuffer, ComPtr<ID3D12Resource>& texture, D3D12_RESOURCE_STATES resourceStates)
{
	uptr<uint8_t[]> ddsData;
	std::vector<D3D12_SUBRESOURCE_DATA> vSubresources;
	DDS_ALPHA_MODE ddsAlphaMode = DDS_ALPHA_MODE_UNKNOWN;
	bool bIsCubeMap = false;

	HRESULT hResult = DirectX::LoadDDSTextureFromFileEx(device.Get(), fileName.c_str(), 0, D3D12_RESOURCE_FLAG_NONE, DDS_LOADER_DEFAULT, &texture, ddsData, vSubresources, &ddsAlphaMode, &bIsCubeMap);
	assert(SUCCEEDED(hResult));

	D3D12_HEAP_PROPERTIES heapPropertiesDesc{};
	heapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapPropertiesDesc.CreationNodeMask = 1;
	heapPropertiesDesc.VisibleNodeMask = 1;

	//	D3D12_RESOURCE_DESC resourceDesc = texture->GetDesc();
	//	UINT nSubResources = resourceDesc.DepthOrArraySize * resourceDesc.MipLevels;
	UINT nSubResources = (UINT)vSubresources.size();
	//	UINT64 nBytes = 0;
	//	device->GetCopyableFootprints(&resourceDesc, 0, nSubResources, 0, NULL, NULL, NULL, &nBytes);
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

	hResult = device->CreateCommittedResource(&heapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&uploadBuffer));
	assert(SUCCEEDED(hResult));

	//UINT nSubResources = (UINT)vSubresources.size();
	//D3D12_SUBRESOURCE_DATA *pd3dSubResourceData = new D3D12_SUBRESOURCE_DATA[nSubResources];
	//for (UINT i = 0; i < nSubResources; i++) pd3dSubResourceData[i] = vSubresources.at(i);

	//	std::vector<D3D12_SUBRESOURCE_DATA>::pointer ptr = &vSubresources[0];

	//리소스 데이터를 Upload Heap에 복사하고 이를 texture 리소스에 복사
	::UpdateSubresources(cmdList.Get(), texture.Get(), uploadBuffer.Get(), 0, 0, nSubResources, &vSubresources[0]);

	SynchronizeResourceTransition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, resourceStates);
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

	return texture;
}

void SynchronizeResourceTransition(RComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
	D3D12_RESOURCE_BARRIER resourceBarrier;
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = resource.Get();
	resourceBarrier.Transition.StateBefore = stateBefore;
	resourceBarrier.Transition.StateAfter = stateAfter;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	cmdList->ResourceBarrier(1, &resourceBarrier);
}



//////////////////* Math *//////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int RoundToNearestMultiple(int value, int multiple)
{
	int remainder = value % multiple;
	int result{};

	if (remainder <= multiple / 2) {
		return value - remainder;
	}

	return value + (multiple - remainder);
}


//////////////////* I/O *//////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ReadUnityBinaryString(FILE* file, std::string& token)
{
	BYTE length{};
	token.resize(256);

	UINT nReads = 0;
	nReads = (UINT)::fread(&length, sizeof(BYTE), 1, file);
	nReads = (UINT)::fread(token.data(), sizeof(char), length, file);
	token.resize(length);

	return(nReads);
}

int ReadIntegerFromFile(FILE* file)
{
	int nValue{};
	UINT nReads = (UINT)::fread(&nValue, sizeof(int), 1, file);
	return(nValue);
}

float ReadFloatFromFile(FILE* file)
{
	float fValue{};
	UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, file);
	return(fValue);
}

UINT ReadStringFromFile(FILE* file, std::string& token)
{
	BYTE length{};
	UINT nReads{};

	token.resize(256);
	nReads = (UINT)::fread(&length, sizeof(BYTE), 1, file);
	nReads = (UINT)::fread(token.data(), sizeof(char), length, file);
	token.resize(length);

	if (nReads == 0) {
		throw std::runtime_error("Failed read file!\n");
	}
	return nReads;
}



//////////////////* Others *//////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrintMessage(const char* message)
{
	MessageBoxA(NULL, message, "Message", MB_OK | MB_ICONERROR);
}


void PrintErrorBlob(RComPtr<ID3DBlob> errorBlob)
{
#if defined(_DEBUG)
	if (errorBlob) {
		PrintMessage(static_cast<const char*>(errorBlob->GetBufferPointer()));
	}
	else {

	}
#endif
}

XMVECTOR SetUnitVector(float x, float y, float z)
{
	XMVECTOR xmvOne = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR xmvZero = XMVectorZero();

	return XMVector3Normalize(XMVectorSet(x, y, z, 0.f));
}

XMVECTOR RandomUnitVectorOnSphere()
{
	return SetUnitVector(RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f));
}

XMVECTOR RandomUnitVectorOnDom()
{
	return SetUnitVector(RandF(-1.0f, 1.0f), RandF(.0f, 1.0f), RandF(-1.0f, 1.0f));
}

void LoadTextureNames(std::vector<std::string>& out, const std::string& folder)
{
	for (const auto& file : std::filesystem::directory_iterator(folder)) {
		std::string fileName = file.path().filename().string();
		fileName.erase(fileName.size() - 4); // remove .dds
		out.emplace_back(fileName);
	}
}