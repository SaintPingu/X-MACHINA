#include "stdafx.h"
#include "Shader.h"
#include "DXGIMgr.h"

#include "Model.h"
#include "Object.h"
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









#pragma region Shader
Shader::~Shader()
{
	assert(mIsClosed);
}

void Shader::Create(DXGI_FORMAT dsvFormat, bool isClose)
{
	assert(!mIsClosed);

	mPipelineStateDesc.pRootSignature = scene->GetRootSignature().Get();
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

	const auto& rtvFormats = dxgi->GetRtvFormats();
	mPipelineStateDesc.NumRenderTargets = rtvFormats.size();
	for (UINT i = 0; i < mPipelineStateDesc.NumRenderTargets; ++i) {
		mPipelineStateDesc.RTVFormats[i] = rtvFormats[i];
	}

	if (mPipelineStates.empty()) {
		mPipelineStates.resize(1);
	}

	HRESULT hResult = device->CreateGraphicsPipelineState(&mPipelineStateDesc, IID_PPV_ARGS(&mPipelineStates[0]));
	AssertHResult(hResult);

	mIsClosed = false;
	if (isClose) {
		Close();
	}
}

void Shader::Set(int pipelineStateIndex)
{
	SetPipelineState(pipelineStateIndex);
	UpdateShaderVars();
}



void Shader::SetPipelineState(int pipelineStateIndex)
{
	assert(mPipelineStates.size() >= pipelineStateIndex + 1);

	cmdList->SetPipelineState(mPipelineStates[pipelineStateIndex].Get());
}

D3D12_INPUT_LAYOUT_DESC Shader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = nullptr;
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
	rasterizerDesc.DepthBiasClamp = 0.f;
	rasterizerDesc.SlopeScaledDepthBias = 0.f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
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

D3D12_SHADER_BYTECODE Shader::CreateVertexShader()
{
	throw std::runtime_error("not assigned vertex shader!");
}

D3D12_SHADER_BYTECODE Shader::CreatePixelShader()
{
	throw std::runtime_error("not assigned pixel shader!");
}


D3D12_SHADER_BYTECODE Shader::CompileShaderFile(const std::wstring& fileName, LPCSTR shaderName, LPCSTR shaderProfile, ComPtr<ID3DBlob>& shaderBlob)
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

void Shader::Close()
{
	mIsClosed = true;
	mVSBlob = nullptr;
	mPSBlob = nullptr;
	if (mPipelineStateDesc.InputLayout.pInputElementDescs) {
		delete[] mPipelineStateDesc.InputLayout.pInputElementDescs;
	}
}
#pragma endregion





#pragma region WireShader
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

D3D12_RASTERIZER_DESC WireShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.f;
	rasterizerDesc.SlopeScaledDepthBias = 0.f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_SHADER_BYTECODE WireShader::CreateVertexShader()
{
#ifdef READ_COMPILED_SHADER
	//return Shader::ReadCompiledShaderFile(L"VShader_Wired.cso", mVSBlob);
	return Shader::CompileShaderFile(L"VShader_Wired.hlsl", "VSWired", "vs_5_1", mVSBlob);
#else
	return Shader::CompileShaderFile(L"VShader_Wired.hlsl", "VSWired", "vs_5_1", mVSBlob);
#endif
}

D3D12_SHADER_BYTECODE WireShader::CreatePixelShader()
{
#ifdef READ_COMPILED_SHADER
	//return Shader::ReadCompiledShaderFile(L"PShader_Wired.cso", mPSBlob);
	return Shader::CompileShaderFile(L"PShader_Wired.hlsl", "PSWired", "ps_5_1", mPSBlob);
#else
	return Shader::CompileShaderFile(L"PShader_Wired.hlsl", "PSWired", "ps_5_1", mPSBlob);
#endif  
#pragma endregion

}





#pragma region InstShader
InstShader::~InstShader()
{
	ReleaseShaderVars();
}

void InstShader::SetColor(const Vec3& color)
{
	for (size_t i = 0; i < mObjects.size(); ++i) {
		mSBMap_Inst[i].Color = Vec4(color.x, color.y, color.z, 1.f);
	}
}

void InstShader::Start()
{
	for (auto& object : mObjects) {
		object->Start();
	}
}


void InstShader::Update()
{
	for (auto& object : mObjects) {
		if (object->IsActive()) {
			object->Update();
		}
	}
}
void InstShader::Render()
{
	mMesh->RenderInstanced(mObjects.size());
}


void InstShader::BuildObjects(size_t instCnt, rsptr<const Mesh> mesh)
{
	mObjects.resize(instCnt);

	for (auto& object : mObjects) {
		object = std::make_shared<GameObject>();
	}

	mMesh = mesh;
	CreateShaderVars();
}

void InstShader::SetShaderResourceView()
{
	cmdList->SetGraphicsRootShaderResourceView(scene->GetRootParamIndex(RootParam::Instancing), mSB_Inst->GetGPUVirtualAddress());
}

void InstShader::CreateShaderVars()
{
	D3DUtil::CreateBufferResource(NULL, sizeof(InstBuff) * mObjects.size(), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, mSB_Inst);
	mSB_Inst->Map(0, NULL, (void**)&mSBMap_Inst);
}

void InstShader::UpdateShaderVars()
{
	SetShaderResourceView();

	for (int j = 0; j < mObjects.size(); j++) {
		mSBMap_Inst[j].LocalTransform = Matrix4x4::Transpose(mObjects[j]->GetWorldTransform());
	}
}

void InstShader::ReleaseShaderVars()
{
	if (mSB_Inst) {
		mSB_Inst->Unmap(0, NULL);
		mSB_Inst = nullptr;
	}
}

D3D12_INPUT_LAYOUT_DESC InstShader::CreateInputLayout()
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

D3D12_SHADER_BYTECODE InstShader::CreateVertexShader()
{
#ifdef READ_COMPILED_SHADER
	return Shader::ReadCompiledShaderFile(L"VShader_Instance.cso", mVSBlob);
#else
	return Shader::CompileShaderFile(L"VShader_Instance.hlsl", "VSInstancing", "vs_5_1", mVSBlob);
#endif
}

D3D12_SHADER_BYTECODE InstShader::CreatePixelShader()
{
#ifdef READ_COMPILED_SHADER
	return Shader::ReadCompiledShaderFile(L"PShader_Instance.cso", mPSBlob);
#else
	return Shader::CompileShaderFile(L"PShader_Instance.hlsl", "PSInstancing", "ps_5_1", mPSBlob);
#endif
}
#pragma endregion






#pragma region EffectShader
void EffectShader::Update()
{
	for (auto& [begin, duration] : mActiveGroups) {
		duration += DeltaTime();
		if (duration >= mMaxLifeTime) {
			mTimeOvers.emplace_back(begin);
			continue;
		}

		size_t index = begin;
		size_t end = begin + mCountPerGroup;
		for (; index < end; ++index) {
			mObjects[index]->Update();
		}
	}

	for (size_t index : mTimeOvers) {
		mActiveGroups.erase(index);
	}
	mTimeOvers.clear();
}

void EffectShader::Render()
{
	mMesh->RenderInstanced(mActiveGroups.size() * mCountPerGroup);
}


void EffectShader::BuildObjects(size_t groupCount, size_t countPerGroup, rsptr<const ModelObjectMesh> mesh)
{
	mGroupSize = groupCount;
	mCountPerGroup = countPerGroup;
	mActiveGroups.reserve(mGroupSize);
	mTimeOvers.reserve(mGroupSize);

	mObjects.resize(groupCount * countPerGroup);
	mObjectScripts.resize(mObjects.size());
	for (int i = 0; i < mObjects.size(); ++i) {
		mObjects[i] = std::make_shared<GameObject>();
		mObjectScripts[i] = mObjects[i]->AddComponent<Script_Fragment>();
	}

	mMesh = mesh;
	CreateShaderVars();
}

void EffectShader::SetActive(const Vec3& pos)
{
	if (mActiveGroups.size() >= mGroupSize) {
		return;
	}

	for (size_t i = 0; i < mGroupSize; ++i) {
		size_t index = i * mCountPerGroup;

		if (mActiveGroups.find(index) == mActiveGroups.end()) {
			mActiveGroups.insert(std::make_pair(index, 0.f));

			size_t end = index + mCountPerGroup;
			for (; index < end; ++index) {
				mObjectScripts[index]->Active(pos);
			}

			return;
		}
	}
}


void EffectShader::UpdateShaderVars()
{
	SetShaderResourceView();

	size_t i = 0;
	for (auto& [begin, duration] : mActiveGroups) {
		size_t index = begin;
		size_t end = begin + mCountPerGroup;
		for (; index < end; ++index, ++i) {
			mSBMap_Inst[i].LocalTransform = Matrix4x4::Transpose(mObjects[index]->GetWorldTransform());

			Vec3 color = mObjectScripts[i]->GetColor();
			mSBMap_Inst[i].Color = Vec4(color.x, color.y, color.z, 1.f);
		}
	}
}
#pragma endregion





#pragma region TexturedEffectShader
TexturedEffectShader::TexturedEffectShader()
	: mMaterial(std::make_shared<Material>())
{

}

void TexturedEffectShader::SetTexture(rsptr<Texture> texture)
{
	mMaterial->SetTexture(texture);
}


void TexturedEffectShader::Render()
{
	EffectShader::Render();
}


void TexturedEffectShader::UpdateShaderVars()
{
	mMaterial->UpdateShaderVars();
	cmdList->SetGraphicsRootShaderResourceView(scene->GetRootParamIndex(RootParam::Instancing), mSB_Inst->GetGPUVirtualAddress());

	size_t i = 0;
	for (auto& [begin, duration] : mActiveGroups) {
		size_t index = begin;
		size_t end = begin + mCountPerGroup;
		for (; index < end; ++index, ++i) {
			mSBMap_Inst[i].LocalTransform = Matrix4x4::Transpose(mObjects[index]->GetWorldTransform());
		}
	}
}

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
	return Shader::CompileShaderFile(L"VShader_TextureInstance.hlsl", "VSTextureInstancing", "vs_5_1", mVSBlob);
}

D3D12_SHADER_BYTECODE TexturedEffectShader::CreatePixelShader()
{
	return Shader::CompileShaderFile(L"PShader_TextureInstance.hlsl", "PSTextureInstancing", "ps_5_1", mPSBlob);
}
#pragma endregion





#pragma region StaticShader
void StaticShader::Create()
{
	Shader::Create();
	BuildObjects();
}



void SmallExpEffectShader::BuildObjects()
{
	constexpr int kToFloat{ 10 };

	constexpr size_t kGroupCount{ 200 };
	constexpr size_t kCntPerGroup{ 40 };
	constexpr float kDuration{ 3.f };
	constexpr float kSize{ 0.4f };
	constexpr int kMinSpeed{ 10 * kToFloat };
	constexpr int kMaxSpeed{ 30 * kToFloat };
	constexpr float kRotationSpeed{ 100.f };
	constexpr Vec3 kRotationAxis{ 1.f,1.f,1.f };

	std::uniform_int_distribution uid{ kMinSpeed, kMaxSpeed };

	SetLifeTime(kDuration);
	SetTexture(scene->GetTexture("Metal02"));

	sptr<ModelObjectMesh> mesh = std::make_shared<ModelObjectMesh>();
	mesh->CreateCubeMesh(kSize, kSize, kSize, true);
	EffectShader::BuildObjects(kGroupCount, kCntPerGroup, mesh);

	size_t i{};
	for (auto& object : mObjects) {
		float movingSpeed = static_cast<float>(uid(Math::dre)) / kToFloat;
		Vec3 movingDir{};
		XMStoreFloat3(&movingDir, RandVectorOnSphere());
		const auto& script = object->GetComponent<Script_Fragment>();
		script->Start();

		object->SetFlyable(true);
		script->SetMovingDir(movingDir);
		script->SetMovingSpeed(movingSpeed * 2.f);
		script->SetRotationAxis(kRotationAxis);
		script->SetRotationSpeed(kRotationSpeed);

		++i;
	}
}



void BigExpEffectShader::BuildObjects()
{
	constexpr int kToFloat{ 10 };

	constexpr size_t kGroupCount{ 150 };
	constexpr size_t kCntPerGroup{ 200 };
	constexpr float kDuration{ 6.f };
	constexpr float kSize{ 2.f };
	constexpr int kMinSpeed{ 5 * kToFloat };
	constexpr int kMaxSpeed{ 20 * kToFloat };
	constexpr float kRotationSpeed{ 100.f };
	constexpr Vec3 kRotationAxis{ 1.f,1.f,1.f };

	std::uniform_int_distribution uid{ kMinSpeed, kMaxSpeed };

	SetLifeTime(kDuration);
	SetTexture(scene->GetTexture("Metal02"));

	sptr<ModelObjectMesh> mesh = std::make_shared<ModelObjectMesh>();
	mesh->CreateCubeMesh(kSize, kSize, kSize, true);
	EffectShader::BuildObjects(kGroupCount, kCntPerGroup, mesh);

	size_t i{};
	for (auto& object : mObjects) {
		float movingSpeed = static_cast<float>(uid(Math::dre)) / kToFloat;
		Vec3 movingDir{};
		XMStoreFloat3(&movingDir, RandVectorOnSphere());
		const auto& script = object->GetComponent<Script_Fragment>();
		script->Start();

		object->SetFlyable(true);
		script->SetMovingDir(movingDir);
		script->SetMovingSpeed(movingSpeed * 6.f);
		script->SetRotationAxis(kRotationAxis);
		script->SetRotationSpeed(kRotationSpeed);

		++i;
	}
}
#pragma endregion






#pragma region BulletShader
void BulletShader::BuildObjects(rsptr<const MasterModel> model, const Object* owner)
{
	constexpr size_t kBufferSize = 100;

	mMesh = model->GetMesh();
	mObjects.resize(kBufferSize);
	mObjectScripts.resize(mObjects.size());

	for (size_t i = 0; i < kBufferSize; ++i) {
		mObjects[i] = std::make_shared<GameObject>();
		mObjects[i]->SetModel(model);
		mObjects[i]->AddComponent<Rigidbody>();

		mObjectScripts[i] = mObjects[i]->AddComponent<Script_Bullet>();
		mObjectScripts[i]->SetOwner(owner);
	}

	CreateShaderVars();
}

void BulletShader::SetLifeTime(float bulletLifeTime)
{
	for (auto& script : mObjectScripts) {
		script->SetLifeTime(bulletLifeTime);
	}
}

void BulletShader::SetDamage(float damage)
{
	for (auto& script : mObjectScripts) {
		script->SetDamage(damage);
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
		scene->UpdateObjectGrid(bullet.get(), false);
	}
}

void BulletShader::Render()
{
	mMesh->RenderInstanced(mBuffer.size());
}

void BulletShader::FireBullet(const Vec3& pos, const Vec3& dir, const Vec3& up, float speed)
{
	sptr<GameObject> bulletObject{};
	int idx{};
	for (; idx < mObjects.size(); ++idx) {
		if (!mObjects[idx]->IsActive()) {
			bulletObject = mObjects[idx];
			break;
		}
	}

	if (!bulletObject)
	{
		return;
	}

	mObjectScripts[idx]->Fire(pos, dir, up, speed);
	mBuffer.emplace_back(bulletObject);
}


void BulletShader::UpdateShaderVars()
{
	cmdList->SetGraphicsRootShaderResourceView(scene->GetRootParamIndex(RootParam::Instancing), mSB_Inst->GetGPUVirtualAddress());

	int i{};
	for (auto it = mBuffer.begin(); it != mBuffer.end(); ) {
		auto& object = *it;
		if (!object->IsActive()) {
			it = mBuffer.erase(it);
			continue;
		}

		mSBMap_Inst[i].LocalTransform = Matrix4x4::Transpose(object->GetWorldTransform());
		++i;
		++it;
	}
}
#pragma endregion





#pragma region TexturedShader
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
	return Shader::CompileShaderFile(L"VShader_Standard.hlsl", "VS_Standard", "vs_5_1", mVSBlob);
}

D3D12_SHADER_BYTECODE TexturedShader::CreatePixelShader()
{
	//return Shader::CompileShaderFile(L"PShader_Standard.hlsl", "PS_Standard", "ps_5_1", mPSBlob);
	return Shader::CompileShaderFile(L"PShader_MRT.hlsl", "PSTexturedLightingToMultipleRTs", "ps_5_1", mPSBlob);
}
#pragma endregion





#pragma region ObjectInstShader
D3D12_SHADER_BYTECODE ObjectInstShader::CreateVertexShader()
{
	return Shader::CompileShaderFile(L"VShader_StandardInstance.hlsl", "VS_StandardInstance", "vs_5_1", mVSBlob);
}
#pragma endregion






#pragma region TransparentShader
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
#pragma endregion





#pragma region TerrainShader
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

D3D12_RASTERIZER_DESC TerrainShader::CreateRasterizerState()
{
	return Shader::CreateRasterizerState();

	// WIREFRAME //
	//D3D12_RASTERIZER_DESC rasterizerDesc{};
	//rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	//rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	//rasterizerDesc.FrontCounterClockwise = FALSE;
	//rasterizerDesc.DepthBias = 0;
	//rasterizerDesc.DepthBiasClamp = 0.f;
	//rasterizerDesc.SlopeScaledDepthBias = 0.f;
	//rasterizerDesc.DepthClipEnable = TRUE;
	//rasterizerDesc.MultisampleEnable = FALSE;
	//rasterizerDesc.AntialiasedLineEnable = FALSE;
	//rasterizerDesc.ForcedSampleCount = 0;
	//rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	//return rasterizerDesc;
}

D3D12_SHADER_BYTECODE TerrainShader::CreateVertexShader()
{
	return Shader::CompileShaderFile(L"VShader_Terrain.hlsl", "VSTerrain", "vs_5_1", mVSBlob);
}


D3D12_SHADER_BYTECODE TerrainShader::CreatePixelShader()
{
	return Shader::CompileShaderFile(L"PShader_Terrain.hlsl", "PSTerrain", "ps_5_1", mPSBlob);
}
#pragma endregion





#pragma region SkyBoxShader
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
	return Shader::CompileShaderFile(L"VShader_Skybox.hlsl", "VSSkyBox", "vs_5_1", mVSBlob);
}

D3D12_SHADER_BYTECODE SkyBoxShader::CreatePixelShader()
{
	return Shader::CompileShaderFile(L"PShader_Skybox.hlsl", "PSSkyBox", "ps_5_1", mPSBlob);
}
#pragma endregion






#pragma region WaterShader
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


D3D12_SHADER_BYTECODE WaterShader::CreateVertexShader()
{
	return Shader::CompileShaderFile(L"VShader_Water.hlsl", "VSWater", "vs_5_1", mVSBlob);
}


D3D12_SHADER_BYTECODE WaterShader::CreatePixelShader()
{
	return Shader::CompileShaderFile(L"PShader_Water.hlsl", "PSWater", "ps_5_1", mPSBlob);
}
#pragma endregion






#pragma region PostProcessingShader
PostProcessingShader::PostProcessingShader()
{
	mRtvCnt = dxgi->GetRtvFormats().size() - 1;
	mRtvFormats = dxgi->GetRtvFormats().data() + 1;
}


void PostProcessingShader::CreateResourcesAndRtvsSrvs(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
{
	CreateTextureResources();
	CreateSrvs();
	CreateRtvs(rtvHandle);
}

void PostProcessingShader::OnPrepareRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle)
{
	constexpr int kRenderTargetCnt = 1;

	int resourceCnt = mTextures.size();
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> allRtvHandle(kRenderTargetCnt + resourceCnt);

	// 후면 버퍼 (SV_TARGET[0])
	allRtvHandle.front() = rtvHandles[0];
	cmdList->ClearRenderTargetView(rtvHandles[0], Colors::White, 0, NULL);

	// (SV_TARGET[1] ~ SV_TARGET[n])
	for (int i = 0; i < resourceCnt; ++i) {
		D3DUtil::ResourceTransition(mTextures[i]->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mRtvHandles[i];
		cmdList->ClearRenderTargetView(rtvHandle, Colors::White, 0, NULL);
		allRtvHandle[kRenderTargetCnt + i] = rtvHandle;
	}

	cmdList->OMSetRenderTargets(kRenderTargetCnt + resourceCnt, allRtvHandle.data(), FALSE, dsvHandle);
}

void PostProcessingShader::OnPostRenderTarget()
{
	for (const auto& texture : mTextures) {
		D3DUtil::ResourceTransition(texture->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
	}
}

void PostProcessingShader::Render()
{
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->DrawInstanced(6, 1, 0, 0);
}

void PostProcessingShader::UpdateShaderVars()
{
	// SRV의 GPU Descriptor Handle을 Set한다.
	mTextures[0]->UpdateShaderVars();
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
	return Shader::CompileShaderFile(L"VShader_Post.hlsl", "VSPostProcessing", "vs_5_1", mVSBlob);
}

D3D12_SHADER_BYTECODE PostProcessingShader::CreatePixelShader()
{
	return Shader::CompileShaderFile(L"PShader_Post.hlsl", "PSPostProcessing", "ps_5_1", mPSBlob);
}

void PostProcessingShader::CreateTextureResources()
{
	mTextures.resize(mRtvCnt);

	D3D12_CLEAR_VALUE clearValue = { DXGI_FORMAT_R8G8B8A8_UNORM, { 1.f, 1.f, 1.f, 1.f } };
	for (UINT i = 0; i < mRtvCnt; ++i)
	{
		clearValue.Format = mRtvFormats[i];
		mTextures[i] = std::make_shared<Texture>(D3DResource::Texture2D);
		mTextures[i]->CreateTextureResource(gkFrameBufferWidth, gkFrameBufferHeight, mRtvFormats[i], D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &clearValue);
	}
}

void PostProcessingShader::CreateSrvs()
{
	CreateShaderVars();

	for (UINT i = 0; i < mRtvCnt; ++i) {
		scene->CreateShaderResourceView(mTextures[i].get(), 0);
		mTextures[i]->SetRootParamIndex(scene->GetRootParamIndex(RootParam::RenderTarget));
	}
}

void PostProcessingShader::CreateRtvs(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	mRtvHandles.resize(mRtvCnt);

	for (UINT i = 0; i < mRtvCnt; ++i) {
		rtvDesc.Format = mRtvFormats[i];
		ComPtr<ID3D12Resource> textureResource = mTextures[i]->GetResource();
		device->CreateRenderTargetView(textureResource.Get(), &rtvDesc, rtvHandle);
		mRtvHandles[i] = rtvHandle;
		rtvHandle.ptr += dxgi->GetRtvDescriptorIncSize();
	}
}
#pragma endregion





#pragma region TextureToScreenShader
D3D12_SHADER_BYTECODE TextureToScreenShader::CreateVertexShader()
{
	return Shader::CompileShaderFile(L"VShader_Rect.hlsl", "VSScreen", "vs_5_1", mVSBlob);
}

D3D12_SHADER_BYTECODE TextureToScreenShader::CreatePixelShader()
{
	return Shader::CompileShaderFile(L"PShader_Rect.hlsl", "PSScreenRectSamplingTextured", "ps_5_1", mPSBlob);
}
#pragma endregion






#pragma region BillboardShader
D3D12_RASTERIZER_DESC BillboardShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = -8000;
	rasterizerDesc.DepthBiasClamp = 0.f;
	rasterizerDesc.SlopeScaledDepthBias = 0.f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
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
	return Shader::CompileShaderFile(L"VShader_Billboard.hlsl", "VSBillboard", "vs_5_1", mVSBlob);
}

D3D12_SHADER_BYTECODE BillboardShader::CreatePixelShader()
{
	return Shader::CompileShaderFile(L"PShader_Billboard.hlsl", "PSBillboard", "ps_5_1", mPSBlob);
}




D3D12_SHADER_BYTECODE SpriteShader::CreateVertexShader()
{
	return Shader::CompileShaderFile(L"VShader_Billboard.hlsl", "VSSprite", "vs_5_1", mVSBlob);
}
#pragma endregion






#pragma region CanvasShader
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
	rasterizerDesc.DepthBias = 0.f;
	rasterizerDesc.DepthBiasClamp = 0.f;
	rasterizerDesc.SlopeScaledDepthBias = 0.f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
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
	return Shader::CompileShaderFile(L"VShader_Canvas.hlsl", "VSCanvas", "vs_5_1", mVSBlob);
}

D3D12_SHADER_BYTECODE CanvasShader::CreatePixelShader()
{
	return Shader::CompileShaderFile(L"PShader_Canvas.hlsl", "PSCanvas", "ps_5_1", mPSBlob);
}
#pragma endregion
