#include "stdafx.h"
#include "DDSTextureLoader12.h"
#include "DXGIMgr.h"



#pragma region Functions
void PrintMsgBox(const char* message)
{
	::MessageBoxA(nullptr, message, "Message", MB_OK | MB_ICONERROR);
}


void PrintErrorBlob(RComPtr<ID3DBlob> errorBlob)
{
#if defined(_DEBUG)
	if (errorBlob) {
		PrintMsgBox(static_cast<const char*>(errorBlob->GetBufferPointer()));
	}
#endif
}

Vector RandVectorOnSphere()
{
	return GetUnitVector(Math::RandF(-1.f, 1.f), Math::RandF(-1.f, 1.f), Math::RandF(-1.f, 1.f));
}
Vector RandVectorOnDom()
{
	return GetUnitVector(Math::RandF(-1.f, 1.f), Math::RandF(0.f, 1.f), Math::RandF(-1.f, 1.f));
}
#pragma endregion




#pragma region Namespace
namespace D3DUtil {
	void CreateBufferResource(const void* data, size_t byteSize, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceStates, ComPtr<ID3D12Resource>& uploadBuffer, ComPtr<ID3D12Resource>& buffer)
	{
		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type                 = heapType;
		heapProperties.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask     = 1;
		heapProperties.VisibleNodeMask      = 1;

		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment          = 0;
		resourceDesc.Width              = static_cast<UINT64>(byteSize);
		resourceDesc.Height             = 1;
		resourceDesc.DepthOrArraySize   = 1;
		resourceDesc.MipLevels          = 1;
		resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count   = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

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

		HRESULT hResult = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, resourceInitialStates, nullptr, IID_PPV_ARGS(&buffer));
		AssertHResult(hResult);

		if (data) {
			switch (heapType) {
			case D3D12_HEAP_TYPE_DEFAULT:
			{
				heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
				hResult = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
				AssertHResult(hResult);

				if (uploadBuffer) {
					D3D12_RANGE readRange = { 0, 0 };
					UINT8* bufferDataBegin{};

					uploadBuffer->Map(0, &readRange, (void**)&bufferDataBegin);
					::memcpy(bufferDataBegin, data, byteSize);
					uploadBuffer->Unmap(0, nullptr);

					cmdList->CopyResource(buffer.Get(), uploadBuffer.Get());
				}

				D3DUtil::ResourceTransition(buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, resourceStates);
			}
			break;
			case D3D12_HEAP_TYPE_UPLOAD:
			{
				D3D12_RANGE readRange = { 0, 0 };
				UINT8* bufferDataBegin = nullptr;

				buffer->Map(0, &readRange, (void**)&bufferDataBegin);
				::memcpy(bufferDataBegin, data, byteSize);
				buffer->Unmap(0, nullptr);
				break;
			}

			default:
				break;
			}
		}
	}




	void CreateVertexBufferViews(std::vector<D3D12_VERTEX_BUFFER_VIEW>& out, size_t vertexCount, const VertexBufferViews& bufferViews)
	{
		D3D12_VERTEX_BUFFER_VIEW view{};

		if (bufferViews.VertexBuffer) {
			CreateVertexBufferView<Vec3>(view, vertexCount, bufferViews.VertexBuffer);
			out.push_back(view);
		}

		if (bufferViews.NormalBuffer) {
			CreateVertexBufferView<Vec3>(view, vertexCount, bufferViews.NormalBuffer);
			out.push_back(view);
		}

		if (bufferViews.UV0Buffer) {
			CreateVertexBufferView<Vec2>(view, vertexCount, bufferViews.UV0Buffer);
			out.push_back(view);
		}

		if (bufferViews.UV1Buffer) {
			CreateVertexBufferView<Vec2>(view, vertexCount, bufferViews.UV1Buffer);
			out.push_back(view);
		}

		if (bufferViews.TangentBuffer) {
			CreateVertexBufferView<Vec3>(view, vertexCount, bufferViews.TangentBuffer);
			out.push_back(view);
		}

		if (bufferViews.BiTangentBuffer) {
			CreateVertexBufferView<Vec3>(view, vertexCount, bufferViews.BiTangentBuffer);
			out.push_back(view);
		}

		if (bufferViews.BoneIndexBuffer) {
			CreateVertexBufferView<XMINT4>(view, vertexCount, bufferViews.BoneIndexBuffer);
			out.push_back(view);
		}

		if (bufferViews.BoneWeightBuffer) {
			CreateVertexBufferView<Vec4>(view, vertexCount, bufferViews.BoneWeightBuffer);
			out.push_back(view);
		}
	}




	void CreateTextureResourceFromDDSFile(const std::wstring& fileName, ComPtr<ID3D12Resource>& uploadBuffer, ComPtr<ID3D12Resource>& texture, D3D12_RESOURCE_STATES resourceStates)
	{
		uptr<uint8_t[]> ddsData;
		std::vector<D3D12_SUBRESOURCE_DATA> subResources;
		DDS_ALPHA_MODE ddsAlphaMode = DDS_ALPHA_MODE_UNKNOWN;
		bool bIsCubeMap{ false };

		HRESULT hResult = DirectX::LoadDDSTextureFromFileEx(device.Get(), fileName.c_str(), 0, D3D12_RESOURCE_FLAG_NONE, DDS_LOADER_DEFAULT, &texture, ddsData, subResources, &ddsAlphaMode, &bIsCubeMap);
		AssertHResult(hResult);

		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type                 = D3D12_HEAP_TYPE_UPLOAD;
		heapProperties.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask     = 1;
		heapProperties.VisibleNodeMask      = 1;

		const UINT kSubResourceCnt = (UINT)subResources.size();
		const UINT64 kBuffSize = ::GetRequiredIntermediateSize(texture.Get(), 0, kSubResourceCnt);

		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER; //Upload Heap에는 텍스쳐를 생성할 수 없음
		resourceDesc.Alignment          = 0;
		resourceDesc.Width              = kBuffSize;
		resourceDesc.Height             = 1;
		resourceDesc.DepthOrArraySize   = 1;
		resourceDesc.MipLevels          = 1;
		resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count   = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

		hResult = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
		AssertHResult(hResult);

		//리소스 데이터를 Upload Heap에 복사하고 이를 texture 리소스에 복사
		::UpdateSubresources(cmdList.Get(), texture.Get(), uploadBuffer.Get(), 0, 0, kSubResourceCnt, &subResources[0]);

		D3DUtil::ResourceTransition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, resourceStates);
	}

	ComPtr<ID3D12Resource> CreateTexture2DResource(UINT width, UINT height, UINT elements, UINT mipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS resourceFlags, D3D12_RESOURCE_STATES resourceStates, Vec4 clearColor)
	{
		ComPtr<ID3D12Resource> texture{};

		D3D12_CLEAR_VALUE optimizedClearValue{};
		D3D12_CLEAR_VALUE* pOptimizedClearValue = nullptr;

		// add depth buffer
		if (resourceFlags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) {
			float color[4] = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
			optimizedClearValue = CD3DX12_CLEAR_VALUE(dxgiFormat, color);
			pOptimizedClearValue = &optimizedClearValue;
		}

		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type                 = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask     = 1;
		heapProperties.VisibleNodeMask      = 1;

		D3D12_RESOURCE_DESC textureResourceDesc{};
		textureResourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureResourceDesc.Alignment          = 0;
		textureResourceDesc.Width              = width;
		textureResourceDesc.Height             = height;
		textureResourceDesc.DepthOrArraySize   = elements;
		textureResourceDesc.MipLevels          = mipLevels;
		textureResourceDesc.Format             = dxgiFormat;
		textureResourceDesc.SampleDesc.Count   = 1;
		textureResourceDesc.SampleDesc.Quality = 0;
		textureResourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		textureResourceDesc.Flags              = resourceFlags;

		HRESULT hResult = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &textureResourceDesc, resourceStates, pOptimizedClearValue, IID_PPV_ARGS(&texture));
		AssertHResult(hResult);

		return texture;
	}

	D3D12_RESOURCE_BARRIER ResourceTransition(RComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
	{
		D3D12_RESOURCE_BARRIER resourceBarrier{};
		resourceBarrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceBarrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceBarrier.Transition.pResource   = resource.Get();
		resourceBarrier.Transition.StateBefore = stateBefore;
		resourceBarrier.Transition.StateAfter  = stateAfter;
		resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		cmdList->ResourceBarrier(1, &resourceBarrier);
		return resourceBarrier;
	}

	D3D12_SHADER_BYTECODE CompileShaderFile(const std::wstring& fileName, LPCSTR shaderName, LPCSTR shaderProfile, ComPtr<ID3DBlob>& shaderBlob)
	{
		std::wstring path = L"shaders/" + fileName;
		UINT nCompileFlags = 0;
#if defined(_DEBUG)
		nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		ComPtr<ID3DBlob> errMsg{};
		::D3DCompileFromFile(path.data(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, shaderName, shaderProfile, nCompileFlags, 0, &shaderBlob, &errMsg);

#if defined(_DEBUG)
		//PrintErrorBlob(errMsg);
#endif

		D3D12_SHADER_BYTECODE shaderByteCode{};
		shaderByteCode.BytecodeLength = shaderBlob->GetBufferSize();
		shaderByteCode.pShaderBytecode = shaderBlob->GetBufferPointer();

		return shaderByteCode;
	}

	D3D12_SHADER_BYTECODE ReadCompiledShaderFile(const std::wstring& fileName, ComPtr<ID3DBlob>& shaderBlob)
	{
		std::wstring filePath = L"shaders/cso/" + fileName;

		FILE* file{};
		::_wfopen_s(&file, filePath.data(), L"rb");
		::fseek(file, 0, SEEK_END);
		int fileSize = ::ftell(file);

		BYTE* byteCode = new BYTE[fileSize];
		::rewind(file);
		UINT byteSize = (UINT)::fread(byteCode, sizeof(BYTE), fileSize, file);
		::fclose(file);

		D3D12_SHADER_BYTECODE shaderByteCode{};
		if (!shaderBlob) {
			HRESULT hResult = D3DCreateBlob(byteSize, &shaderBlob);
			AssertHResult(hResult);
			::memcpy(shaderBlob->GetBufferPointer(), byteCode, byteSize);
			shaderByteCode.BytecodeLength = shaderBlob->GetBufferSize();
			shaderByteCode.pShaderBytecode = shaderBlob->GetBufferPointer();
			delete[] byteCode;
		}
		else {
			shaderByteCode.BytecodeLength = byteSize;
			shaderByteCode.pShaderBytecode = byteCode;
		}

		return shaderByteCode;
	}
}
#pragma endregion





#pragma region Class
void MyBoundingOrientedBox::Transform(const Vec4x4& transform)
{
	const Matrix kMatrix = _MATRIX(transform);
	const Vector kRotation = XMQuaternionRotationMatrix(_MATRIX(transform));

	XMStoreFloat4(&Orientation, kRotation);
	XMStoreFloat3(&Center, XMVector3Transform(_VECTOR(mOriginCenter), kMatrix));
}

void MyBoundingSphere::Transform(const Vec4x4& transform)
{
	Center = Matrix4x4::Multiply(transform, mOriginCenter);
}

bool MyBoundingSphere::IntersectBoxes(const std::vector<MyBoundingOrientedBox*>& boxes) const
{
	for (auto& box : boxes) {
		if (Intersects(*box)) {
			return true;
		}
	}

	return false;
}
#pragma endregion

DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& fileName, int lineNumber)
	:
	mErrorCode(hr),
	mFunctionName(functionName),
	mFilename(fileName),
	mLineNumber(lineNumber)
{

}

std::wstring DxException::ToString() const
{
	_com_error err(mErrorCode);
	std::wstring msg = err.ErrorMessage();

	return mFunctionName + L" failed in " + mFilename + L"; line " + std::to_wstring(mLineNumber) + L"; error: " + msg;
}
