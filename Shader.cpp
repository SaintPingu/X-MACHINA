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









// [ Shader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Shader::Shader()
{
}

Shader::~Shader()
{
	ReleaseShaderVariables();

	assert(mIsClosed);
}

D3D12_SHADER_BYTECODE Shader::CreateVertexShader()
{
	D3D12_SHADER_BYTECODE shaderByteCode{};
	shaderByteCode.BytecodeLength = 0;
	shaderByteCode.pShaderBytecode = nullptr;

	return shaderByteCode;
}

D3D12_SHADER_BYTECODE Shader::CreatePixelShader()
{
	D3D12_SHADER_BYTECODE shaderByteCode{};
	shaderByteCode.BytecodeLength = 0;
	shaderByteCode.pShaderBytecode = nullptr;

	return shaderByteCode;
}

D3D12_SHADER_BYTECODE Shader::CompileShaderFromFile(const std::wstring& fileName, LPCSTR shaderName, LPCSTR shaderProfile, ComPtr<ID3DBlob>& shaderBlob)
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

D3D12_SHADER_BYTECODE Shader::ReadCompiledShaderFile(const std::wstring& fileName, ComPtr<ID3DBlob>& shaderBlob)
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

D3D12_INPUT_LAYOUT_DESC Shader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = NULL;
	inputLayoutDesc.NumElements = 0;

	return inputLayoutDesc;
}

D3D12_RASTERIZER_DESC Shader::CreateRasterizerState()
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

D3D12_DEPTH_STENCIL_DESC Shader::CreateDepthStencilState()
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

D3D12_BLEND_DESC Shader::CreateBlendState()
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

void Shader::CreateShader()
{
	CreateShader(1, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);
}

void Shader::CreateShader(UINT renderTargetCnt, DXGI_FORMAT* rtvFormats, DXGI_FORMAT dsvFormat)
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

void Shader::CreateShaderVariables()
{
}

void Shader::UpdateShaderVariables()
{
}

void Shader::ReleaseShaderVariables()
{
}

void Shader::OnPrepareRender(int pipelineStateIndex)
{
	assert(mPipelineStates.size() >= pipelineStateIndex + 1);

	cmdList->SetPipelineState(mPipelineStates[pipelineStateIndex].Get());
}

void Shader::Render(int pipelineStateIndex)
{
	OnPrepareRender(pipelineStateIndex);
	UpdateShaderVariables();
}

void Shader::Close()
{
	mIsClosed = true;
	mVertexShaderBlob = nullptr;
	mPixelShaderBlob = nullptr;
	if (mPipelineStateDesc.InputLayout.pInputElementDescs) {
		delete[] mPipelineStateDesc.InputLayout.pInputElementDescs;
	}
}














// [ WireShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WireShader::WireShader()
{

}

WireShader::~WireShader()
{

}

void WireShader::CreateShader()
{
	DXGI_FORMAT dxgiRtvFormats[2] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM };
	Shader::CreateShader(2, dxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	Close();
}

D3D12_RASTERIZER_DESC WireShader::CreateRasterizerState()
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

D3D12_INPUT_LAYOUT_DESC WireShader::CreateInputLayout()
{
	UINT nInputElementDescs = 1;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = nInputElementDescs;

	return inputLayoutDesc;
}

D3D12_SHADER_BYTECODE WireShader::CreateVertexShader()
{
#ifdef READ_COMPILED_SHADER
	//return Shader::ReadCompiledShaderFile(L"VShader_Wired.cso", mVertexShaderBlob);
	return Shader::CompileShaderFromFile(L"VShader_Wired.hlsl", "VSWired", "vs_5_1", mVertexShaderBlob);
#else
	return Shader::CompileShaderFromFile(L"VShader_Wired.hlsl", "VSWired", "vs_5_1", mVertexShaderBlob);
#endif
}

D3D12_SHADER_BYTECODE WireShader::CreatePixelShader()
{
#ifdef READ_COMPILED_SHADER
	//return Shader::ReadCompiledShaderFile(L"PShader_Wired.cso", mPixelShaderBlob);
	return Shader::CompileShaderFromFile(L"PShader_Wired.hlsl", "PSWired", "ps_5_1", mPixelShaderBlob);
#else
	return Shader::CompileShaderFromFile(L"PShader_Wired.hlsl", "PSWired", "ps_5_1", mPixelShaderBlob);
#endif
}










// [ InstancingShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
InstancingShader::InstancingShader()
{

}

InstancingShader::~InstancingShader()
{

}

D3D12_INPUT_LAYOUT_DESC InstancingShader::CreateInputLayout()
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

D3D12_SHADER_BYTECODE InstancingShader::CreateVertexShader()
{
#ifdef READ_COMPILED_SHADER
	return Shader::ReadCompiledShaderFile(L"VShader_Instance.cso", mVertexShaderBlob);
#else
	return Shader::CompileShaderFromFile(L"VShader_Instance.hlsl", "VSInstancing", "vs_5_1", mVertexShaderBlob);
#endif
}

D3D12_SHADER_BYTECODE InstancingShader::CreatePixelShader()
{
#ifdef READ_COMPILED_SHADER
	return Shader::ReadCompiledShaderFile(L"PShader_Instance.cso", mPixelShaderBlob);
#else
	return Shader::CompileShaderFromFile(L"PShader_Instance.hlsl", "PSInstancing", "ps_5_1", mPixelShaderBlob);
#endif
}

void InstancingShader::CreateShader()
{
	DXGI_FORMAT dxgiRtvFormats[5] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };
	Shader::CreateShader(5, dxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	Shader::Close();
	/*Shader::CreateShader();
	Close();*/
}

void InstancingShader::CreateShaderVariables()
{
	::CreateBufferResource(NULL, sizeof(INSTANCE_BUFFER) * mObjects.size(), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, mInstBuffer);
	mInstBuffer->Map(0, NULL, (void**)&mMappedObjects);
}

void InstancingShader::UpdateShaderVariables()
{
	cmdList->SetGraphicsRootShaderResourceView(crntScene->GetRootParamIndex(RootParam::Instancing), mInstBuffer->GetGPUVirtualAddress());
	for (int j = 0; j < mObjects.size(); j++) {
		::memcpy(&mMappedObjects[j].mLocalTransform, &Matrix4x4::Transpose(mObjects[j]->GetWorldTransform()), sizeof(Vec4x4));
	}
}

void InstancingShader::ReleaseShaderVariables()
{
	if (mInstBuffer) mInstBuffer->Unmap(0, NULL);
	mInstBuffer = nullptr;
}

void InstancingShader::Render()
{
	Shader::Render();
	UpdateShaderVariables();
	mMesh->Render(0, mObjects.size());
}


void InstancingShader::Start()
{
	for (auto& object : mObjects) {
		object->Start();
	}
}


void InstancingShader::Update()
{
	for (auto& object : mObjects) {
		if (object->IsActive()) {
			object->Update();
		}
	}
}

void InstancingShader::SetColor(const Vec3& color)
{
	for (size_t i = 0; i < mObjects.size(); ++i) {
		mMappedObjects[i].mColor = Vec4(color.x, color.y, color.z, 1.0f);
	}
}

void InstancingShader::BuildObjects(size_t instanceCount, rsptr<const Mesh> mesh)
{
	mObjects.resize(instanceCount);

	for (auto& object : mObjects) {
		object = std::make_shared<GameObject>();
	}

	mMesh = mesh;
	CreateShaderVariables();
}













// [ EffectShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EffectShader::EffectShader()
{

}

EffectShader::~EffectShader()
{

}


void EffectShader::UpdateShaderVariables()
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

void EffectShader::Render()
{
	Shader::Render();
	UpdateShaderVariables();
	mMesh->Render(0, mActiveGroups.size() * mCountPerGroup);
}

void EffectShader::Update()
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

void EffectShader::SetColor(size_t i, const Vec3& color)
{
	if (i < mObjects.size()) {
		mObjects[i]->GetComponent<Script_Fragment>()->SetColor(color);
	}
}

size_t EffectShader::GetGroupBegin(size_t index)
{
	assert(index < mObjects.size());
	return mGroupSize * index;
}


void EffectShader::BuildObjects(size_t groupCount, size_t countPerGroup, rsptr<const ModelObjectMesh> mesh)
{
	mGroupSize = groupCount;
	mCountPerGroup = countPerGroup;
	mActiveGroups.reserve(mGroupSize);
	timeOvers.reserve(mGroupSize);

	mObjects.resize(groupCount * countPerGroup);
	for (auto& object : mObjects) {
		object = std::make_shared<GameObject>();
		object->AddComponent<Script_Fragment>();
	}

	mMesh = mesh;
	CreateShaderVariables();
}


void EffectShader::SetActive(const Vec3& pos)
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


// [ StatiShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void StatiShader::Create()
{
	CreateShader();
	BuildObjects();
}




// [ SmallExpEffectShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SmallExpEffectShader::BuildObjects()
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

	sptr<ModelObjectMesh> mesh = std::make_shared<ModelObjectMesh>(size, size, size, true);
	EffectShader::BuildObjects(groupCount, countPerGroup, mesh);

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


// [ BigExpEffectShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BigExpEffectShader::BuildObjects()
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

	sptr<ModelObjectMesh> mesh = std::make_shared<ModelObjectMesh>(size, size, size, true);
	EffectShader::BuildObjects(groupCount, countPerGroup, mesh);

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



// [ TexturedEffectShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3D12_INPUT_LAYOUT_DESC TexturedEffectShader::CreateInputLayout()
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

D3D12_SHADER_BYTECODE TexturedEffectShader::CreateVertexShader()
{
	return Shader::CompileShaderFromFile(L"VShader_TextureInstance.hlsl", "VSTextureInstancing", "vs_5_1", mVertexShaderBlob);
}

D3D12_SHADER_BYTECODE TexturedEffectShader::CreatePixelShader()
{
	return Shader::CompileShaderFromFile(L"PShader_TextureInstance.hlsl", "PSTextureInstancing", "ps_5_1", mPixelShaderBlob);
}


void TexturedEffectShader::UpdateShaderVariables()
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


void TexturedEffectShader::Render()
{
	EffectShader::Render();
}



// [ BulletShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BulletShader::BulletShader()
{

}

BulletShader::~BulletShader()
{

}

void BulletShader::BuildObjects(size_t bufferSize, rsptr<const MasterModel> model, const Object* owner)
{
	mMesh = model->GetMesh();
	mObjects.resize(bufferSize);

	for (size_t i = 0; i < bufferSize; ++i) {
		mObjects[i] = std::make_shared<GameObject>();
		const auto& script = mObjects[i]->AddComponent<Script_Bullet>();

		mObjects[i]->SetModel(model);
		script->SetOwner(owner);

		mObjects[i]->AddComponent<Rigidbody>();
	}

	CreateShaderVariables();
}

void BulletShader::SetLifeTime(float bulletLifeTime)
{
	for (auto& object : mObjects) {
		object->GetComponent<Script_Bullet>()->SetLifeTime(bulletLifeTime);
	}
}

void BulletShader::SetDamage(float damage)
{
	for (auto& object : mObjects) {
		object->GetComponent<Script_Bullet>()->SetDamage(damage);
	}
}

void BulletShader::FireBullet(const Vec3& pos, const Vec3& dir, const Vec3& up, float speed)
{
	sptr<GameObject> bulletObject{};
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

void BulletShader::UpdateShaderVariables()
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

void BulletShader::Start()
{
	for (auto& bullet : mObjects) {
		bullet->Start();
	}
}

void BulletShader::Update()
{
	for (auto& bullet : mBuffer) {
		bullet->Update();
	}
}

void BulletShader::Render()
{
	Shader::Render();
	UpdateShaderVariables();
	mMesh->Render(0, mBuffer.size());
}



// [ CIlluminatedShdaer ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IlluminatedShader::IlluminatedShader()
{
}

IlluminatedShader::~IlluminatedShader()
{
}

D3D12_INPUT_LAYOUT_DESC IlluminatedShader::CreateInputLayout()
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

D3D12_SHADER_BYTECODE IlluminatedShader::CreateVertexShader()
{
#ifdef READ_COMPILED_SHADER
	return Shader::ReadCompiledShaderFile(L"VShader_Lighting.cso", mVertexShaderBlob);
#else
	return Shader::CompileShaderFromFile(L"VShader_Lighting.hlsl", "VSLighting", "vs_5_1", mVertexShaderBlob);
#endif
}

D3D12_SHADER_BYTECODE IlluminatedShader::CreatePixelShader()
{
#ifdef READ_COMPILED_SHADER
	return Shader::ReadCompiledShaderFile(L"PShader_Lighting.cso", mPixelShaderBlob);
#else
	return Shader::CompileShaderFromFile(L"PShader_Lighting.hlsl", "PSLighting", "ps_5_1", mPixelShaderBlob);
#endif
}

void IlluminatedShader::CreateShader()
{
	mPipelineStates.resize(2);

	Shader::CreateShader();

	mPipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	HRESULT hResult = device->CreateGraphicsPipelineState(&mPipelineStateDesc, IID_PPV_ARGS(&mPipelineStates[1]));

	Close();
}

void IlluminatedShader::Render(int pipelineStateIndex)
{
	OnPrepareRender(pipelineStateIndex);
}







// [ TexturedShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TexturedShader::TexturedShader()
{
}

TexturedShader::~TexturedShader()
{
}

void TexturedShader::CreateShader()
{
	DXGI_FORMAT dxgiRtvFormats[5] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };
	Shader::CreateShader(5, dxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	//Shader::CreateShader();
	Shader::Close();
}

D3D12_INPUT_LAYOUT_DESC TexturedShader::CreateInputLayout()
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

D3D12_SHADER_BYTECODE TexturedShader::CreateVertexShader()
{
	return Shader::CompileShaderFromFile(L"VShader_Standard.hlsl", "VS_Standard", "vs_5_1", mVertexShaderBlob);
}

D3D12_SHADER_BYTECODE TexturedShader::CreatePixelShader()
{
	//return Shader::CompileShaderFromFile(L"PShader_Standard.hlsl", "PS_Standard", "ps_5_1", mPixelShaderBlob);
	return Shader::CompileShaderFromFile(L"PShader_MRT.hlsl", "PSTexturedLightingToMultipleRTs", "ps_5_1", mPixelShaderBlob);
}



// [ ObjectInstancingShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3D12_SHADER_BYTECODE ObjectInstancingShader::CreateVertexShader()
{
	return Shader::CompileShaderFromFile(L"VShader_StandardInstance.hlsl", "VS_StandardInstance", "vs_5_1", mVertexShaderBlob);
}



// [ TransparentShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3D12_DEPTH_STENCIL_DESC TransparentShader::CreateDepthStencilState()
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
D3D12_BLEND_DESC TransparentShader::CreateBlendState()
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



// [ TerrainShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TerrainShader::TerrainShader()
{

}
TerrainShader::~TerrainShader()
{

}

void TerrainShader::CreateShader()
{
	DXGI_FORMAT dxgiRtvFormats[5] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };
	Shader::CreateShader(5, dxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	Shader::Close();
}

D3D12_RASTERIZER_DESC TerrainShader::CreateRasterizerState()
{
	return Shader::CreateRasterizerState();

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

D3D12_INPUT_LAYOUT_DESC TerrainShader::CreateInputLayout()
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


D3D12_SHADER_BYTECODE TerrainShader::CreateVertexShader()
{
	return Shader::CompileShaderFromFile(L"VShader_Terrain.hlsl", "VSTerrain", "vs_5_1", mVertexShaderBlob);
}


D3D12_SHADER_BYTECODE TerrainShader::CreatePixelShader()
{
	return Shader::CompileShaderFromFile(L"PShader_Terrain.hlsl", "PSTerrain", "ps_5_1", mPixelShaderBlob);
}



// [ SkyBoxShader ] // 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SkyBoxShader::SkyBoxShader()
{
}

SkyBoxShader::~SkyBoxShader()
{
}


void SkyBoxShader::CreateShader()
{
	Shader::CreateShader();
	Shader::Close();
}


D3D12_INPUT_LAYOUT_DESC SkyBoxShader::CreateInputLayout()
{
	UINT nInputElementDescs = 1;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = nInputElementDescs;

	return inputLayoutDesc;
}

D3D12_DEPTH_STENCIL_DESC SkyBoxShader::CreateDepthStencilState()
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

D3D12_SHADER_BYTECODE SkyBoxShader::CreateVertexShader()
{
	return Shader::CompileShaderFromFile(L"VShader_Skybox.hlsl", "VSSkyBox", "vs_5_1", mVertexShaderBlob);
}

D3D12_SHADER_BYTECODE SkyBoxShader::CreatePixelShader()
{
	return Shader::CompileShaderFromFile(L"PShader_Skybox.hlsl", "PSSkyBox", "ps_5_1", mPixelShaderBlob);
}





// [ WaterShader ] // 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3D12_DEPTH_STENCIL_DESC WaterShader::CreateDepthStencilState()
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


D3D12_BLEND_DESC WaterShader::CreateBlendState()
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

D3D12_SHADER_BYTECODE WaterShader::CreateVertexShader()
{
	return Shader::CompileShaderFromFile(L"VShader_Water.hlsl", "VSWater", "vs_5_1", mVertexShaderBlob);
}


D3D12_SHADER_BYTECODE WaterShader::CreatePixelShader()
{
	return Shader::CompileShaderFromFile(L"PShader_Water.hlsl", "PSWater", "ps_5_1", mPixelShaderBlob);
}





// [ SkyBoxShader ] // // [ PostProcessingShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PostProcessingShader::PostProcessingShader()
{
}

PostProcessingShader::~PostProcessingShader()
{
}

D3D12_DEPTH_STENCIL_DESC PostProcessingShader::CreateDepthStencilState()
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

D3D12_SHADER_BYTECODE PostProcessingShader::CreateVertexShader()
{
	return Shader::CompileShaderFromFile(L"VShader_Post.hlsl", "VSPostProcessing", "vs_5_1", mVertexShaderBlob);
}

D3D12_SHADER_BYTECODE PostProcessingShader::CreatePixelShader()
{
	return Shader::CompileShaderFromFile(L"PShader_Post.hlsl", "PSPostProcessing", "ps_5_1", mPixelShaderBlob);
}

void PostProcessingShader::CreateShader(UINT renderTargetCnt, DXGI_FORMAT* rtvFormats, DXGI_FORMAT dsvFormat)
{
	Shader::CreateShader(renderTargetCnt, rtvFormats, dsvFormat);
	Shader::Close();
}

// texture resource를 생성한다 (ID3D12Resource)
void PostProcessingShader::CreateTextureResources(UINT renderTargetCnt, DXGI_FORMAT* dxgiFormats)
{
	mTextures.resize(renderTargetCnt);

	D3D12_CLEAR_VALUE clearValue = { DXGI_FORMAT_R8G8B8A8_UNORM, { 1.0f, 1.0f, 1.0f, 1.0f } };
	for (UINT i = 0; i < renderTargetCnt; ++i)
	{
		clearValue.Format = dxgiFormats[i];
		mTextures[i] = std::make_shared<Texture>(RESOURCE_TEXTURE2D);
		mTextures[i]->CreateTexture(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, dxgiFormats[i], D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &clearValue);
	}
}

// resource의 SRV Descriptor를 생성한다. (ID3D12Device::CreateShaderResourceView)
void PostProcessingShader::CreateSrvs(UINT renderTargetCnt)
{
	CreateShaderVariables();
	for (UINT i = 0; i < renderTargetCnt; ++i) {
		crntScene->CreateShaderResourceView(mTextures[i].get(), 0);
		mTextures[i]->SetRootParamIndex(crntScene->GetRootParamIndex(RootParam::RenderTarget));
	}
}

// resource의 RTV Descriptor를 생성한다. (ID3D12Device::CreateRenderTargetView)
void PostProcessingShader::CreateRtvs(UINT renderTargetCnt, DXGI_FORMAT* dxgiFormats, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
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

void PostProcessingShader::CreateResourcesAndRtvsSrvs(UINT renderTargetCnt, DXGI_FORMAT* dxgiFormats, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
{
	// texture resource를 생성하고 이에 대한 SRV와 RTV를 생성한다
	CreateTextureResources(renderTargetCnt, dxgiFormats);
	CreateSrvs(renderTargetCnt);
	CreateRtvs(renderTargetCnt, dxgiFormats, rtvHandle);
}

// 각 RTV의 handle을 받아와 Clear하고 이들을 OutputMerger에 Set한다.
void PostProcessingShader::OnPrepareRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle)
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

void PostProcessingShader::OnPostRenderTarget()
{
	for (const auto& texture : mTextures) {
		::SynchronizeResourceTransition(texture->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
	}
}

void PostProcessingShader::Render()
{
	Shader::Render();

	// SRV의 GPU Descriptor Handle의 시작 주소로 Set한다.
	mTextures[0]->UpdateShaderVariables();

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->DrawInstanced(6, 1, 0, 0);
}



// [ TextureToFullScreenShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TextureToFullScreenShader::TextureToFullScreenShader()
{
}

TextureToFullScreenShader::~TextureToFullScreenShader()
{
}

D3D12_SHADER_BYTECODE TextureToFullScreenShader::CreateVertexShader()
{
	return(Shader::CompileShaderFromFile(L"VShader_Rect.hlsl", "VSScreen", "vs_5_1", mVertexShaderBlob));
}

D3D12_SHADER_BYTECODE TextureToFullScreenShader::CreatePixelShader()
{
	return(Shader::CompileShaderFromFile(L"PShader_Rect.hlsl", "PSScreenRectSamplingTextured", "ps_5_1", mPixelShaderBlob));
}


// [ BillboardShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BillboardShader::CreateShader()
{
	DXGI_FORMAT dxgiRtvFormats[5] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };
	Shader::CreateShader(5, dxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	Shader::Close();
}

D3D12_RASTERIZER_DESC BillboardShader::CreateRasterizerState()
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

D3D12_BLEND_DESC BillboardShader::CreateBlendState()
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

D3D12_SHADER_BYTECODE BillboardShader::CreateVertexShader()
{
	return(Shader::CompileShaderFromFile(L"VShader_Billboard.hlsl", "VSBillboard", "vs_5_1", mVertexShaderBlob));
}

D3D12_SHADER_BYTECODE BillboardShader::CreatePixelShader()
{
	return(Shader::CompileShaderFromFile(L"PShader_Billboard.hlsl", "PSBillboard", "ps_5_1", mPixelShaderBlob));
}




D3D12_SHADER_BYTECODE SpriteShader::CreateVertexShader()
{
	return(Shader::CompileShaderFromFile(L"VShader_Billboard.hlsl", "VSSprite", "vs_5_1", mVertexShaderBlob));
}







void CanvasShader::CreateShader()
{
	DXGI_FORMAT dxgiRtvFormats[5] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };
	Shader::CreateShader(5, dxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	Close();
}

D3D12_INPUT_LAYOUT_DESC CanvasShader::CreateInputLayout()
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

D3D12_RASTERIZER_DESC CanvasShader::CreateRasterizerState()
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

D3D12_BLEND_DESC CanvasShader::CreateBlendState()
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

D3D12_SHADER_BYTECODE CanvasShader::CreateVertexShader()
{
	return(Shader::CompileShaderFromFile(L"VShader_Canvas.hlsl", "VSCanvas", "vs_5_1", mVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CanvasShader::CreatePixelShader()
{
	return(Shader::CompileShaderFromFile(L"PShader_Canvas.hlsl", "PSCanvas", "ps_5_1", mPixelShaderBlob));
}