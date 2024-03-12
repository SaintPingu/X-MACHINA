#pragma once

#pragma region EnumClass
enum class RootSignatureType {
	Graphics,
	Compute,
};
#pragma endregion

// util of ID3D12RootSignature
// root parameter�� �����Ͽ� �����ϰ� ����ϵ��� �ϰ� ������ �ּ�ȭ�Ѵ�.
class RootSignature {
protected:
	RootSignatureType mType{};
	ComPtr<ID3D12RootSignature> mRootSignature{};

	std::vector<D3D12_ROOT_PARAMETER>	mParams{};		// all root parameters
	std::vector<D3D12_DESCRIPTOR_RANGE> mRanges{};		// all root ranges
	std::unordered_map<RootParam, UINT> mParamMap{};	// [mPrams]�� �ε��� ��

public:
	RootSignature();
	virtual ~RootSignature() = default;

	RComPtr<ID3D12RootSignature> Get() const { return mRootSignature; }
	UINT GetRootParamIndex(RootParam param) const { return mParamMap.at(param); }

public:
	// ��Ī�� �߰��� �ϳ��� root parameter�� ���� �̸����� ������ �� �ְ� �Ѵ�.
	void AddAlias(RootParam origin, RootParam alias) { mParamMap[alias] = mParamMap[origin]; }

	// add root parameter
	void Push(RootParam param, D3D12_ROOT_PARAMETER_TYPE paramType, UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visibility, UINT num32BitValues = 0);
	// add root parameter with descriptor table and range
	void PushTable(RootParam param, D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT shaderRegister, UINT registerSpace, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility);

protected:
	// ID3D12Device::CreateRootSignature
	virtual void Create() abstract;

	// [mPrams]�� �ε����� �����Ѵ�.
	void ParamMapping(RootParam param);
};


// for rendering
class GraphicsRootSignature : public RootSignature {
public:
	GraphicsRootSignature();
	virtual ~GraphicsRootSignature() = default;

public:
	void CreateDefaultGraphicsRootSignature();
	void CreateParticleGraphicsRootSignature();

protected:
	virtual void Create() override;

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 8> GetStaticSamplers();
};

// for computing
class ComputeRootSignature : public RootSignature {
public:
	ComputeRootSignature();
	virtual ~ComputeRootSignature() = default;

public:
	void CreateDefaultComputeRootSignature();
	void CreateParticleComputeRootSignature();

protected:
	virtual void Create() override;
};
