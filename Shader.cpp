//-----------------------------------------------------------------------------
// File: Shader.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Shader.h"
#include "DXGIMgr.h"

#include "Model.h"
#include "Camera.h"
#include "Mesh.h"
#include "Scene.h"
#include "Timer.h"
#include "Collider.h"
#include "Texture.h"

#include "Script_Fragment.h"
#include "Script_Bullet.h"
#include "Rigidbody.h"

//#define READ_COMPILED_SHADER









// [ CShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CShader::CShader()
{
}

CShader::~CShader()
{
	ReleaseShaderVariables();

	assert(mIsClosed);
}

D3D12_SHADER_BYTECODE CShader::CreateVertexShader()
{
	D3D12_SHADER_BYTECODE shaderByteCode{};
	shaderByteCode.BytecodeLength = 0;
	shaderByteCode.pShaderBytecode = nullptr;

	return shaderByteCode;
}

D3D12_SHADER_BYTECODE CShader::CreatePixelShader()
{
	D3D12_SHADER_BYTECODE shaderByteCode{};
	shaderByteCode.BytecodeLength = 0;
	shaderByteCode.pShaderBytecode = nullptr;

	return shaderByteCode;
}

D3D12_SHADER_BYTECODE CShader::CompileShaderFromFile(const std::wstring& fileName, LPCSTR shaderName, LPCSTR shaderProfile, ComPtr<ID3DBlob>& shaderBlob)
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

D3D12_SHADER_BYTECODE CShader::ReadCompiledShaderFile(const std::wstring& fileName, ComPtr<ID3DBlob>& shaderBlob)
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
		memcpy(shaderBlob->GetBufferPointer(), byteCode, byteSize);
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

D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = NULL;
	inputLayoutDesc.NumElements = 0;

	return inputLayoutDesc;
}

D3D12_RASTERIZER_DESC CShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(rasterizerDesc);
}

D3D12_DEPTH_STENCIL_DESC CShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = 0x00;
	depthStencilDesc.StencilWriteMask = 0x00;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return depthStencilDesc;
}

D3D12_BLEND_DESC CShader::CreateBlendState()
{
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

void CShader::CreateShader()
{
	CreateShader(1, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);
}

void CShader::CreateShader(UINT renderTargetCnt, DXGI_FORMAT* rtvFormats, DXGI_FORMAT dsvFormat)
{
	mIsClosed = false;

	mPipelineStateDesc.pRootSignature = crntScene->GetRootSignature().Get();
	mPipelineStateDesc.VS = CreateVertexShader();
	mPipelineStateDesc.PS = CreatePixelShader();
	mPipelineStateDesc.RasterizerState = CreateRasterizerState();
	mPipelineStateDesc.BlendState = CreateBlendState();
	mPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	mPipelineStateDesc.InputLayout = CreateInputLayout();
	mPipelineStateDesc.SampleMask = UINT_MAX;
	mPipelineStateDesc.PrimitiveTopologyType = GetPrimitiveType();
	mPipelineStateDesc.DSVFormat = dsvFormat;
	mPipelineStateDesc.SampleDesc.Count = 1;
	mPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	mPipelineStateDesc.NumRenderTargets = renderTargetCnt;
	for (UINT i = 0; i < renderTargetCnt; ++i) {
		mPipelineStateDesc.RTVFormats[i] = (rtvFormats) ? rtvFormats[i] : DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	if (mPipelineStates.size() == 0) {
		mPipelineStates.resize(1);
	}

	HRESULT hResult = device->CreateGraphicsPipelineState(&mPipelineStateDesc, IID_PPV_ARGS(&mPipelineStates[0]));
	assert(SUCCEEDED(hResult));
}

void CShader::CreateShaderVariables()
{
}

void CShader::UpdateShaderVariables()
{
}

void CShader::ReleaseShaderVariables()
{
}

void CShader::OnPrepareRender(int pipelineStateIndex)
{
	assert(mPipelineStates.size() >= pipelineStateIndex + 1);

	cmdList->SetPipelineState(mPipelineStates[pipelineStateIndex].Get());
}

void CShader::Render(int pipelineStateIndex)
{
	OnPrepareRender(pipelineStateIndex);
	UpdateShaderVariables();
}

void CShader::Close()
{
	mIsClosed = true;
	mVertexShaderBlob = nullptr;
	mPixelShaderBlob = nullptr;
	if (mPipelineStateDesc.InputLayout.pInputElementDescs) {
		delete[] mPipelineStateDesc.InputLayout.pInputElementDescs;
	}
}














// [ CWireShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CWireShader::CWireShader()
{

}

CWireShader::~CWireShader()
{

}

void CWireShader::CreateShader()
{
	DXGI_FORMAT dxgiRtvFormats[2] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM };
	CShader::CreateShader(2, dxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	Close();
}

D3D12_RASTERIZER_DESC CWireShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(rasterizerDesc);
}

D3D12_INPUT_LAYOUT_DESC CWireShader::CreateInputLayout()
{
	UINT nInputElementDescs = 1;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = nInputElementDescs;

	return inputLayoutDesc;
}

D3D12_SHADER_BYTECODE CWireShader::CreateVertexShader()
{
#ifdef READ_COMPILED_SHADER
	//return CShader::ReadCompiledShaderFile(L"VShader_Wired.cso", mVertexShaderBlob);
	return CShader::CompileShaderFromFile(L"VShader_Wired.hlsl", "VSWired", "vs_5_1", mVertexShaderBlob);
#else
	return CShader::CompileShaderFromFile(L"VShader_Wired.hlsl", "VSWired", "vs_5_1", mVertexShaderBlob);
#endif
}

D3D12_SHADER_BYTECODE CWireShader::CreatePixelShader()
{
#ifdef READ_COMPILED_SHADER
	//return CShader::ReadCompiledShaderFile(L"PShader_Wired.cso", mPixelShaderBlob);
	return CShader::CompileShaderFromFile(L"PShader_Wired.hlsl", "PSWired", "ps_5_1", mPixelShaderBlob);
#else
	return CShader::CompileShaderFromFile(L"PShader_Wired.hlsl", "PSWired", "ps_5_1", mPixelShaderBlob);
#endif
}










// [ CInstancingShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CInstancingShader::CInstancingShader()
{

}

CInstancingShader::~CInstancingShader()
{

}

D3D12_INPUT_LAYOUT_DESC CInstancingShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];
	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = nInputElementDescs;
	return inputLayoutDesc;
}

D3D12_SHADER_BYTECODE CInstancingShader::CreateVertexShader()
{
#ifdef READ_COMPILED_SHADER
	return CShader::ReadCompiledShaderFile(L"VShader_Instance.cso", mVertexShaderBlob);
#else
	return CShader::CompileShaderFromFile(L"VShader_Instance.hlsl", "VSInstancing", "vs_5_1", mVertexShaderBlob);
#endif
}

D3D12_SHADER_BYTECODE CInstancingShader::CreatePixelShader()
{
#ifdef READ_COMPILED_SHADER
	return CShader::ReadCompiledShaderFile(L"PShader_Instance.cso", mPixelShaderBlob);
#else
	return CShader::CompileShaderFromFile(L"PShader_Instance.hlsl", "PSInstancing", "ps_5_1", mPixelShaderBlob);
#endif
}

void CInstancingShader::CreateShader()
{
	DXGI_FORMAT dxgiRtvFormats[5] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };
	CShader::CreateShader(5, dxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	CShader::Close();
	/*CShader::CreateShader();
	Close();*/
}

void CInstancingShader::CreateShaderVariables()
{
	::CreateBufferResource(NULL, sizeof(INSTANCE_BUFFER) * mObjects.size(), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, mInstBuffer);
	mInstBuffer->Map(0, NULL, (void**)&mMappedObjects);
}

void CInstancingShader::UpdateShaderVariables()
{
	cmdList->SetGraphicsRootShaderResourceView(crntScene->GetRootParamIndex(RootParam::Instancing), mInstBuffer->GetGPUVirtualAddress());
	for (int j = 0; j < mObjects.size(); j++) {
		::memcpy(&mMappedObjects[j].mLocalTransform, &Matrix4x4::Transpose(mObjects[j]->GetWorldTransform()), sizeof(Vec4x4));
	}
}

void CInstancingShader::ReleaseShaderVariables()
{
	if (mInstBuffer) mInstBuffer->Unmap(0, NULL);
	mInstBuffer = nullptr;
}

void CInstancingShader::Render()
{
	CShader::Render();
	UpdateShaderVariables();
	mMesh->Render(0, mObjects.size());
}


void CInstancingShader::Start()
{
	for (auto& object : mObjects) {
		object->Start();
	}
}


void CInstancingShader::Update()
{
	for (auto& object : mObjects) {
		if (object->IsActive()) {
			object->Update();
		}
	}
}

void CInstancingShader::SetColor(const Vec3& color)
{
	for (size_t i = 0; i < mObjects.size(); ++i) {
		mMappedObjects[i].mColor = Vec4(color.x, color.y, color.z, 1.0f);
	}
}

void CInstancingShader::BuildObjects(size_t instanceCount, rsptr<const CMesh> mesh)
{
	mObjects.resize(instanceCount);

	for (auto& object : mObjects) {
		object = std::make_shared<CGameObject>();
	}

	mMesh = mesh;
	CreateShaderVariables();
}













// [ CEffectShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CEffectShader::CEffectShader()
{

}

CEffectShader::~CEffectShader()
{

}


void CEffectShader::UpdateShaderVariables()
{
	cmdList->SetGraphicsRootShaderResourceView(crntScene->GetRootParamIndex(RootParam::Instancing), mInstBuffer->GetGPUVirtualAddress());

	size_t i = 0;
	for (auto& [begin, duration] : mActiveGroups) {
		size_t index = begin;
		size_t end = begin + mCountPerGroup;
		for (; index < end; ++index, ++i) {
			::memcpy(&mMappedObjects[i].mLocalTransform, &Matrix4x4::Transpose(mObjects[index]->GetWorldTransform()), sizeof(Vec4x4));
			Vec3 color = mObjects[i]->GetComponent<Script_Fragment>()->GetColor();
			mMappedObjects[i].mColor = Vec4(color.x, color.y, color.z, 1.0f);
		}
	}
}

void CEffectShader::Render()
{
	CShader::Render();
	UpdateShaderVariables();
	mMesh->Render(0, mActiveGroups.size() * mCountPerGroup);
}

void CEffectShader::Update()
{
	for (auto& [begin, duration] : mActiveGroups) {
		duration += DeltaTime();
		if (duration >= mMaxDuration) {
			timeOvers.emplace_back(begin);
			continue;
		}

		size_t index = begin;
		size_t end = begin + mCountPerGroup;
		for (; index < end; ++index) {
			auto& object = mObjects[index];
			object->Update();
		}
	}

	for (size_t index : timeOvers) {
		mActiveGroups.erase(index);
	}
	timeOvers.clear();
}

void CEffectShader::SetColor(size_t i, const Vec3& color)
{
	if (i < mObjects.size()) {
		mObjects[i]->GetComponent<Script_Fragment>()->SetColor(color);
	}
}

size_t CEffectShader::GetGroupBegin(size_t index)
{
	assert(index < mObjects.size());
	return mGroupSize * index;
}


void CEffectShader::BuildObjects(size_t groupCount, size_t countPerGroup, rsptr<const CModelObjectMesh> mesh)
{
	mGroupSize = groupCount;
	mCountPerGroup = countPerGroup;
	mActiveGroups.reserve(mGroupSize);
	timeOvers.reserve(mGroupSize);

	mObjects.resize(groupCount * countPerGroup);
	for (auto& object : mObjects) {
		object = std::make_shared<CGameObject>();
		object->AddComponent<Script_Fragment>();
	}

	mMesh = mesh;
	CreateShaderVariables();
}


void CEffectShader::SetActive(const Vec3& pos)
{
	// groups are full, return nothing
	if (mActiveGroups.size() >= mGroupSize) {
		return;
	}

	for (size_t i = 0; i < mGroupSize; ++i) {
		size_t index = i * mCountPerGroup;

		if (mActiveGroups.find(index) == mActiveGroups.end()) {
			mActiveGroups.insert(std::make_pair(index, 0.0f));

			size_t end = index + mCountPerGroup;
			for (; index < end; ++index) {
				mObjects[index]->GetComponent<Script_Fragment>()->Active(pos);
			}

			return;
		}
	}
}


// [ CStaticShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CStaticShader::Create()
{
	CreateShader();
	BuildObjects();
}




// [ CSmallExpEffectShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSmallExpEffectShader::BuildObjects()
{
	constexpr int toFloat{ 10 };

	constexpr size_t groupCount{ 200 };
	constexpr size_t countPerGroup{ 40 };
	constexpr float duration{ 3.f };
	constexpr float size{ 0.4f };
	constexpr int minSpeed{ 10 * toFloat };
	constexpr int maxSpeed{ 30 * toFloat };
	constexpr float rotationSpeed{ 100.0f };
	constexpr Vec3 RotationAxis{ 1.0f,1.0f,1.0f };

	std::default_random_engine dre;
	std::uniform_int_distribution uid{ minSpeed, maxSpeed };

	SetDuration(duration);
	SetMaterial(crntScene->GetMaterial("Metal02"));

	sptr<CModelObjectMesh> mesh = std::make_shared<CModelObjectMesh>(size, size, size, true);
	CEffectShader::BuildObjects(groupCount, countPerGroup, mesh);

	size_t i{};
	for (auto& object : mObjects) {
		float movingSpeed = static_cast<float>(uid(dre)) / toFloat;
		Vec3 movingDir{};
		XMStoreFloat3(&movingDir, RandomUnitVectorOnSphere());
		auto& script = object->GetComponent<Script_Fragment>();
		script->Start();

		object->SetFlyable(true);
		script->SetMovingDir(movingDir);
		script->SetMovingSpeed(movingSpeed * 2.f);
		script->SetRotationAxis(RotationAxis);
		script->SetRotationSpeed(rotationSpeed);

		++i;
	}
}


// [ CBigExpEffectShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CBigExpEffectShader::BuildObjects()
{
	constexpr int toFloat{ 10 };

	constexpr size_t groupCount{ 150 };
	constexpr size_t countPerGroup{ 200 };
	constexpr float duration{ 6.f };
	constexpr float size{ 2.f };
	constexpr int minSpeed{ 5 * toFloat };
	constexpr int maxSpeed{ 20 * toFloat };
	constexpr float rotationSpeed{ 100.0f };
	constexpr Vec3 RotationAxis{ 1.0f,1.0f,1.0f };

	std::default_random_engine dre;
	std::uniform_int_distribution uid{ minSpeed, maxSpeed };

	SetDuration(duration);
	SetMaterial(crntScene->GetMaterial("Metal02"));

	sptr<CModelObjectMesh> mesh = std::make_shared<CModelObjectMesh>(size, size, size, true);
	CEffectShader::BuildObjects(groupCount, countPerGroup, mesh);

	size_t i{};
	for (auto& object : mObjects) {
		float movingSpeed = static_cast<float>(uid(dre)) / toFloat;
		Vec3 movingDir{};
		XMStoreFloat3(&movingDir, RandomUnitVectorOnSphere());
		auto& script = object->GetComponent<Script_Fragment>();
		script->Start();

		object->SetFlyable(true);
		script->SetMovingDir(movingDir);
		script->SetMovingSpeed(movingSpeed * 6.f);
		script->SetRotationAxis(RotationAxis);
		script->SetRotationSpeed(rotationSpeed);

		++i;
	}
}



// [ CTexturedEffectShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3D12_INPUT_LAYOUT_DESC CTexturedEffectShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];
	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };


	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = nInputElementDescs;
	return inputLayoutDesc;
}

D3D12_SHADER_BYTECODE CTexturedEffectShader::CreateVertexShader()
{
	return CShader::CompileShaderFromFile(L"VShader_TextureInstance.hlsl", "VSTextureInstancing", "vs_5_1", mVertexShaderBlob);
}

D3D12_SHADER_BYTECODE CTexturedEffectShader::CreatePixelShader()
{
	return CShader::CompileShaderFromFile(L"PShader_TextureInstance.hlsl", "PSTextureInstancing", "ps_5_1", mPixelShaderBlob);
}


void CTexturedEffectShader::UpdateShaderVariables()
{
	mMaterial->UpdateShaderVariable();
	cmdList->SetGraphicsRootShaderResourceView(crntScene->GetRootParamIndex(RootParam::Instancing), mInstBuffer->GetGPUVirtualAddress());

	size_t i = 0;
	for (auto& [begin, duration] : mActiveGroups) {
		size_t index = begin;
		size_t end = begin + mCountPerGroup;
		for (; index < end; ++index, ++i) {
			::memcpy(&mMappedObjects[i].mLocalTransform, &Matrix4x4::Transpose(mObjects[index]->GetWorldTransform()), sizeof(Vec4x4));
		}
	}
}


void CTexturedEffectShader::Render()
{
	CEffectShader::Render();
}



// [ CBulletShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBulletShader::CBulletShader()
{

}

CBulletShader::~CBulletShader()
{

}

void CBulletShader::BuildObjects(size_t bufferSize, rsptr<const CMasterModel> model, const CObject* owner)
{
	mMesh = model->GetMesh();
	mObjects.resize(bufferSize);

	for (size_t i = 0; i < bufferSize; ++i) {
		mObjects[i] = std::make_shared<CGameObject>();
		const auto& script = mObjects[i]->AddComponent<Script_Bullet>();

		mObjects[i]->SetModel(model);
		script->SetOwner(owner);

		mObjects[i]->AddComponent<Rigidbody>();
	}

	CreateShaderVariables();
}

void CBulletShader::SetLifeTime(float bulletLifeTime)
{
	for (auto& object : mObjects) {
		object->GetComponent<Script_Bullet>()->SetLifeTime(bulletLifeTime);
	}
}

void CBulletShader::SetDamage(float damage)
{
	for (auto& object : mObjects) {
		object->GetComponent<Script_Bullet>()->SetDamage(damage);
	}
}

void CBulletShader::FireBullet(const Vec3& pos, const Vec3& dir, const Vec3& up, float speed)
{
	sptr<CGameObject> bulletObject{};
	for (auto& object : mObjects)
	{
		if (!object->IsActive())
		{
			bulletObject = object;
			break;
		}
	}

	if (!bulletObject)
	{
		return;
	}

	bulletObject->GetComponent<Script_Bullet>()->Fire(pos, dir, up, speed);
	mBuffer.emplace_back(bulletObject);
}

void CBulletShader::UpdateShaderVariables()
{
	cmdList->SetGraphicsRootShaderResourceView(crntScene->GetRootParamIndex(RootParam::Instancing), mInstBuffer->GetGPUVirtualAddress());

	int i{};
	for (auto it = mBuffer.begin(); it != mBuffer.end(); ) {
		auto& object = *it;
		if (!object->IsActive()) {
			it = mBuffer.erase(it);
			continue;
		}

		::memcpy(&mMappedObjects[i].mLocalTransform, &Matrix4x4::Transpose(object->GetWorldTransform()), sizeof(Vec4x4));
		++i;
		++it;
	}
}

void CBulletShader::Start()
{
	for (auto& bullet : mObjects) {
		bullet->Start();
	}
}

void CBulletShader::Update()
{
	for (auto& bullet : mBuffer) {
		bullet->Update();
	}
}

void CBulletShader::Render()
{
	CShader::Render();
	UpdateShaderVariables();
	mMesh->Render(0, mBuffer.size());
}



// [ CIlluminatedShdaer ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIlluminatedShader::CIlluminatedShader()
{
}

CIlluminatedShader::~CIlluminatedShader()
{
}

D3D12_INPUT_LAYOUT_DESC CIlluminatedShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = nInputElementDescs;

	return inputLayoutDesc;
}

D3D12_SHADER_BYTECODE CIlluminatedShader::CreateVertexShader()
{
#ifdef READ_COMPILED_SHADER
	return CShader::ReadCompiledShaderFile(L"VShader_Lighting.cso", mVertexShaderBlob);
#else
	return CShader::CompileShaderFromFile(L"VShader_Lighting.hlsl", "VSLighting", "vs_5_1", mVertexShaderBlob);
#endif
}

D3D12_SHADER_BYTECODE CIlluminatedShader::CreatePixelShader()
{
#ifdef READ_COMPILED_SHADER
	return CShader::ReadCompiledShaderFile(L"PShader_Lighting.cso", mPixelShaderBlob);
#else
	return CShader::CompileShaderFromFile(L"PShader_Lighting.hlsl", "PSLighting", "ps_5_1", mPixelShaderBlob);
#endif
}

void CIlluminatedShader::CreateShader()
{
	mPipelineStates.resize(2);

	CShader::CreateShader();

	mPipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	HRESULT hResult = device->CreateGraphicsPipelineState(&mPipelineStateDesc, IID_PPV_ARGS(&mPipelineStates[1]));

	Close();
}

void CIlluminatedShader::Render(int pipelineStateIndex)
{
	OnPrepareRender(pipelineStateIndex);
}







// [ CTexturedShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTexturedShader::CTexturedShader()
{
}

CTexturedShader::~CTexturedShader()
{
}

void CTexturedShader::CreateShader()
{
	DXGI_FORMAT dxgiRtvFormats[5] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };
	CShader::CreateShader(5, dxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	//CShader::CreateShader();
	CShader::Close();
}

D3D12_INPUT_LAYOUT_DESC CTexturedShader::CreateInputLayout()
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = nInputElementDescs;

	return inputLayoutDesc;
}

D3D12_SHADER_BYTECODE CTexturedShader::CreateVertexShader()
{
	return CShader::CompileShaderFromFile(L"VShader_Standard.hlsl", "VS_Standard", "vs_5_1", mVertexShaderBlob);
}

D3D12_SHADER_BYTECODE CTexturedShader::CreatePixelShader()
{
	//return CShader::CompileShaderFromFile(L"PShader_Standard.hlsl", "PS_Standard", "ps_5_1", mPixelShaderBlob);
	return CShader::CompileShaderFromFile(L"PShader_MRT.hlsl", "PSTexturedLightingToMultipleRTs", "ps_5_1", mPixelShaderBlob);
}



// [ CObjectInstancingShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3D12_SHADER_BYTECODE CObjectInstancingShader::CreateVertexShader()
{
	return CShader::CompileShaderFromFile(L"VShader_StandardInstance.hlsl", "VS_StandardInstance", "vs_5_1", mVertexShaderBlob);
}



// [ CTransparentShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3D12_DEPTH_STENCIL_DESC CTransparentShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = 0xff;
	depthStencilDesc.StencilWriteMask = 0xff;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return depthStencilDesc;
}
D3D12_BLEND_DESC CTransparentShader::CreateBlendState()
{
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = TRUE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}



// [ CTerrainShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTerrainShader::CTerrainShader()
{

}
CTerrainShader::~CTerrainShader()
{

}

void CTerrainShader::CreateShader()
{
	DXGI_FORMAT dxgiRtvFormats[5] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };
	CShader::CreateShader(5, dxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	CShader::Close();
}

D3D12_RASTERIZER_DESC CTerrainShader::CreateRasterizerState()
{
	return CShader::CreateRasterizerState();

	// WIREFRAME //
	//D3D12_RASTERIZER_DESC rasterizerDesc{};
	//rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	//rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	//rasterizerDesc.FrontCounterClockwise = FALSE;
	//rasterizerDesc.DepthBias = 0;
	//rasterizerDesc.DepthBiasClamp = 0.0f;
	//rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	//rasterizerDesc.DepthClipEnable = TRUE;
	//rasterizerDesc.MultisampleEnable = FALSE;
	//rasterizerDesc.AntialiasedLineEnable = FALSE;
	//rasterizerDesc.ForcedSampleCount = 0;
	//rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	//return(rasterizerDesc);
}

D3D12_INPUT_LAYOUT_DESC CTerrainShader::CreateInputLayout()
{
	UINT nInputElementDescs = 4;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "UVA", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "UVB", 0, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = nInputElementDescs;

	return inputLayoutDesc;
}


D3D12_SHADER_BYTECODE CTerrainShader::CreateVertexShader()
{
	return CShader::CompileShaderFromFile(L"VShader_Terrain.hlsl", "VSTerrain", "vs_5_1", mVertexShaderBlob);
}


D3D12_SHADER_BYTECODE CTerrainShader::CreatePixelShader()
{
	return CShader::CompileShaderFromFile(L"PShader_Terrain.hlsl", "PSTerrain", "ps_5_1", mPixelShaderBlob);
}



// [ CSkyboxShader ] // 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSkyBoxShader::CSkyBoxShader()
{
}

CSkyBoxShader::~CSkyBoxShader()
{
}


void CSkyBoxShader::CreateShader()
{
	CShader::CreateShader();
	CShader::Close();
}


D3D12_INPUT_LAYOUT_DESC CSkyBoxShader::CreateInputLayout()
{
	UINT nInputElementDescs = 1;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = nInputElementDescs;

	return inputLayoutDesc;
}

D3D12_DEPTH_STENCIL_DESC CSkyBoxShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = 0xff;
	depthStencilDesc.StencilWriteMask = 0xff;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return depthStencilDesc;
}

D3D12_SHADER_BYTECODE CSkyBoxShader::CreateVertexShader()
{
	return CShader::CompileShaderFromFile(L"VShader_Skybox.hlsl", "VSSkyBox", "vs_5_1", mVertexShaderBlob);
}

D3D12_SHADER_BYTECODE CSkyBoxShader::CreatePixelShader()
{
	return CShader::CompileShaderFromFile(L"PShader_Skybox.hlsl", "PSSkyBox", "ps_5_1", mPixelShaderBlob);
}





// [ CWaterShader ] // 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3D12_DEPTH_STENCIL_DESC CWaterShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = 0xff;
	depthStencilDesc.StencilWriteMask = 0xff;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return depthStencilDesc;
}


D3D12_BLEND_DESC CWaterShader::CreateBlendState()
{
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

D3D12_SHADER_BYTECODE CWaterShader::CreateVertexShader()
{
	return CShader::CompileShaderFromFile(L"VShader_Water.hlsl", "VSWater", "vs_5_1", mVertexShaderBlob);
}


D3D12_SHADER_BYTECODE CWaterShader::CreatePixelShader()
{
	return CShader::CompileShaderFromFile(L"PShader_Water.hlsl", "PSWater", "ps_5_1", mPixelShaderBlob);
}





// [ CSkyboxShader ] // // [ CPostProcessingShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPostProcessingShader::CPostProcessingShader()
{
}

CPostProcessingShader::~CPostProcessingShader()
{
}

D3D12_DEPTH_STENCIL_DESC CPostProcessingShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = 0x00;
	depthStencilDesc.StencilWriteMask = 0x00;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return depthStencilDesc;
}

D3D12_SHADER_BYTECODE CPostProcessingShader::CreateVertexShader()
{
	return CShader::CompileShaderFromFile(L"VShader_Post.hlsl", "VSPostProcessing", "vs_5_1", mVertexShaderBlob);
}

D3D12_SHADER_BYTECODE CPostProcessingShader::CreatePixelShader()
{
	return CShader::CompileShaderFromFile(L"PShader_Post.hlsl", "PSPostProcessing", "ps_5_1", mPixelShaderBlob);
}

void CPostProcessingShader::CreateShader(UINT renderTargetCnt, DXGI_FORMAT* rtvFormats, DXGI_FORMAT dsvFormat)
{
	CShader::CreateShader(renderTargetCnt, rtvFormats, dsvFormat);
	CShader::Close();
}

// texture resource를 생성한다 (ID3D12Resource)
void CPostProcessingShader::CreateTextureResources(UINT renderTargetCnt, DXGI_FORMAT* dxgiFormats)
{
	mTextures.resize(renderTargetCnt);

	D3D12_CLEAR_VALUE clearValue = { DXGI_FORMAT_R8G8B8A8_UNORM, { 1.0f, 1.0f, 1.0f, 1.0f } };
	for (UINT i = 0; i < renderTargetCnt; ++i)
	{
		clearValue.Format = dxgiFormats[i];
		mTextures[i] = std::make_shared<CTexture>(RESOURCE_TEXTURE2D);
		mTextures[i]->CreateTexture(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, dxgiFormats[i], D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &clearValue);
	}
}

// resource의 SRV Descriptor를 생성한다. (ID3D12Device::CreateShaderResourceView)
void CPostProcessingShader::CreateSrvs(UINT renderTargetCnt)
{
	CreateShaderVariables();
	for (UINT i = 0; i < renderTargetCnt; ++i) {
		crntScene->CreateShaderResourceView(mTextures[i].get(), 0);
		mTextures[i]->SetRootParamIndex(crntScene->GetRootParamIndex(RootParam::RenderTarget));
	}
}

// resource의 RTV Descriptor를 생성한다. (ID3D12Device::CreateRenderTargetView)
void CPostProcessingShader::CreateRtvs(UINT renderTargetCnt, DXGI_FORMAT* dxgiFormats, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	mRtvHandles.resize(renderTargetCnt);

	for (UINT i = 0; i < renderTargetCnt; ++i) {
		rtvDesc.Format = dxgiFormats[i];
		ComPtr<ID3D12Resource> textureResource = mTextures[i]->GetResource();
		device->CreateRenderTargetView(textureResource.Get(), &rtvDesc, rtvHandle);
		mRtvHandles[i] = rtvHandle;
		rtvHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void CPostProcessingShader::CreateResourcesAndRtvsSrvs(UINT renderTargetCnt, DXGI_FORMAT* dxgiFormats, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
{
	// texture resource를 생성하고 이에 대한 SRV와 RTV를 생성한다
	CreateTextureResources(renderTargetCnt, dxgiFormats);
	CreateSrvs(renderTargetCnt);
	CreateRtvs(renderTargetCnt, dxgiFormats, rtvHandle);
}

// 각 RTV의 handle을 받아와 Clear하고 이들을 OutputMerger에 Set한다.
void CPostProcessingShader::OnPrepareRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle)
{
	constexpr int renderTargetCnt = 1;

	int resourceCnt = mTextures.size();
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> allRtvHandle(renderTargetCnt + resourceCnt);

	// 후면 버퍼 (SV_TARGET[0])
	allRtvHandle.front() = rtvHandles[0];
	cmdList->ClearRenderTargetView(rtvHandles[0], Colors::White, 0, NULL);

	// (SV_TARGET[1] ~ SV_TARGET[n])
	for (int i = 0; i < resourceCnt; ++i) {
		::SynchronizeResourceTransition(mTextures[i]->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mRtvHandles[i];
		cmdList->ClearRenderTargetView(rtvHandle, Colors::White, 0, NULL);
		allRtvHandle[renderTargetCnt + i] = rtvHandle;
	}

	cmdList->OMSetRenderTargets(renderTargetCnt + resourceCnt, allRtvHandle.data(), FALSE, dsvHandle);
}

void CPostProcessingShader::OnPostRenderTarget()
{
	for (const auto& texture : mTextures) {
		::SynchronizeResourceTransition(texture->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
	}
}

void CPostProcessingShader::Render()
{
	CShader::Render();

	// SRV의 GPU Descriptor Handle의 시작 주소로 Set한다.
	mTextures[0]->UpdateShaderVariables();

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->DrawInstanced(6, 1, 0, 0);
}



// [ CTextureToFullScreenShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTextureToFullScreenShader::CTextureToFullScreenShader()
{
}

CTextureToFullScreenShader::~CTextureToFullScreenShader()
{
}

D3D12_SHADER_BYTECODE CTextureToFullScreenShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"VShader_Rect.hlsl", "VSScreen", "vs_5_1", mVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CTextureToFullScreenShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"PShader_Rect.hlsl", "PSScreenRectSamplingTextured", "ps_5_1", mPixelShaderBlob));
}


// [ CBillboardShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBillboardShader::CreateShader()
{
	DXGI_FORMAT dxgiRtvFormats[5] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };
	CShader::CreateShader(5, dxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	CShader::Close();
}

D3D12_RASTERIZER_DESC CBillboardShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = -8000;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(rasterizerDesc);
}

D3D12_BLEND_DESC CBillboardShader::CreateBlendState()
{
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

D3D12_SHADER_BYTECODE CBillboardShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"VShader_Billboard.hlsl", "VSBillboard", "vs_5_1", mVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CBillboardShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"PShader_Billboard.hlsl", "PSBillboard", "ps_5_1", mPixelShaderBlob));
}




D3D12_SHADER_BYTECODE CSpriteShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"VShader_Billboard.hlsl", "VSSprite", "vs_5_1", mVertexShaderBlob));
}







void CCanvasShader::CreateShader()
{
	DXGI_FORMAT dxgiRtvFormats[5] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };
	CShader::CreateShader(5, dxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	Close();
}

D3D12_INPUT_LAYOUT_DESC CCanvasShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];
	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = nInputElementDescs;
	return inputLayoutDesc;
}

D3D12_RASTERIZER_DESC CCanvasShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0.0f;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(rasterizerDesc);
}

D3D12_BLEND_DESC CCanvasShader::CreateBlendState()
{
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = TRUE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

D3D12_SHADER_BYTECODE CCanvasShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"VShader_Canvas.hlsl", "VSCanvas", "vs_5_1", mVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CCanvasShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"PShader_Canvas.hlsl", "PSCanvas", "ps_5_1", mPixelShaderBlob));
}






void CMirrorShader::CreateShader()
{
	DXGI_FORMAT dxgiRtvFormats[5] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };

	mPipelineStates.resize(4);
	CShader::CreateShader(5, dxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	CreatePiepeLineStates();
	Close();
}

void CMirrorShader::SetMirrorObject(rsptr<CGameObject> mirror)
{
	mMirror = mirror;

}

D3D12_BLEND_DESC CMirrorShader::MirrorBlendState()
{
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

D3D12_DEPTH_STENCIL_DESC CMirrorShader::MirrorDepthStencilDesc()
{
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable = TRUE;
	depthStencilDesc.StencilReadMask = 0xff;
	depthStencilDesc.StencilWriteMask = 0xff;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return depthStencilDesc;
}

D3D12_DEPTH_STENCIL_DESC CMirrorShader::ReflectDepthStencilDesc()
{
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable = TRUE;
	depthStencilDesc.StencilReadMask = 0xff;
	depthStencilDesc.StencilWriteMask = 0xff;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	return depthStencilDesc;
}
D3D12_RASTERIZER_DESC CMirrorShader::CWCullRasterizerDesc()
{
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = TRUE;
	rasterizerDesc.DepthBias = 0.0f;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(rasterizerDesc);
}

D3D12_BLEND_DESC CMirrorShader::TranspaerntBlendState()
{
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

void CMirrorShader::CreatePiepeLineStates()
{
	HRESULT hResult;

	// 0 : 불투명 오브젝트
	D3D12_GRAPHICS_PIPELINE_STATE_DESC defaultPSO = mPipelineStateDesc;

	// 1 : 투명 오브젝트
	mPSOs[PSO::Transparent] = mPipelineStates[1];
	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPSO = defaultPSO;
	transparentPSO.BlendState = TranspaerntBlendState();
	hResult = device->CreateGraphicsPipelineState(&transparentPSO, IID_PPV_ARGS(&mPSOs[PSO::Transparent]));
	assert(SUCCEEDED(hResult));

	// 2 : 스텐실 버퍼
	mPSOs[PSO::StencilMirror] = mPipelineStates[2];
	D3D12_GRAPHICS_PIPELINE_STATE_DESC stencilPSO = defaultPSO;
	stencilPSO.DepthStencilState = MirrorDepthStencilDesc();
	stencilPSO.BlendState = MirrorBlendState();
	hResult = device->CreateGraphicsPipelineState(&stencilPSO, IID_PPV_ARGS(&mPSOs[PSO::StencilMirror]));
	assert(SUCCEEDED(hResult));

	// 3 : 반사된 물체
	mPSOs[PSO::ReflectObjects] = mPipelineStates[3];
	D3D12_GRAPHICS_PIPELINE_STATE_DESC reflectPSO = defaultPSO;
	reflectPSO.DepthStencilState = ReflectDepthStencilDesc();
	reflectPSO.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	reflectPSO.RasterizerState.FrontCounterClockwise = TRUE;
	hResult = device->CreateGraphicsPipelineState(&reflectPSO, IID_PPV_ARGS(&mPSOs[PSO::ReflectObjects]));
	assert(SUCCEEDED(hResult));
}


D3D12_BLEND_DESC CMirrorShader::CreateBlendState()
{
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

D3D12_DEPTH_STENCIL_DESC CMirrorShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return depthStencilDesc;
}

void CMirrorShader::Render()
{

	Vec3 pos = mMirror->GetPosition();
	Vec3 look = mMirror->GetUp();
	mMirrorPlane = { look.x, look.y, look.z, -((look.x * pos.x) + (look.y * pos.y) + (look.z * pos.z)) };

	mMtxReflect = XMMatrixReflect(XMLoadFloat4(&mMirrorPlane));

	dxgi->ClearStencil();

	//// 거울 렌더링 및 스텐실 버퍼에 쓰기
	cmdList->OMSetStencilRef(1);
	cmdList->SetPipelineState(mPSOs[PSO::StencilMirror].Get());
	if (mMirror) {
		mMirror->Render();
	}

	isRenderReflectObject = true;
	//// 반사된 객체 렌더링
	cmdList->OMSetStencilRef(1);
	cmdList->SetPipelineState(mPSOs[PSO::ReflectObjects].Get());
	crntScene->RenderMirrorObjects(mMirrorPlane);
	isRenderReflectObject = false;

	//// 거울(투명 오브젝트) 렌더링
	cmdList->SetPipelineState(mPSOs[PSO::Transparent].Get());
	if (mMirror) {
		mMirror->Render();
	}

}
