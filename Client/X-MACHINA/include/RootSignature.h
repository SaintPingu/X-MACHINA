#pragma once

#pragma region EnumClass
enum class RootSignatureType {
	Graphics,
	Compute,
};
#pragma endregion

// util of ID3D12RootSignature
// root parameter를 매핑하여 간편하게 사용하도록 하고 오류를 최소화한다.
class RootSignature {
protected:
	RootSignatureType mType{};
	ComPtr<ID3D12RootSignature> mRootSignature{};

	std::vector<D3D12_ROOT_PARAMETER>	mParams{};		// all root parameters
	std::vector<D3D12_DESCRIPTOR_RANGE> mRanges{};		// all root ranges
	std::unordered_map<RootParam, UINT> mParamMap{};	// [mPrams]의 인덱스 맵

public:
	RootSignature();
	virtual ~RootSignature() = default;

	RComPtr<ID3D12RootSignature> Get() const { return mRootSignature; }
	UINT GetRootParamIndex(RootParam param) const { return mParamMap.at(param); }

public:
	// 별칭을 추가해 하나의 root parameter를 여러 이름으로 접근할 수 있게 한다.
	void AddAlias(RootParam origin, RootParam alias) { mParamMap[alias] = mParamMap[origin]; }

	// add root parameter
	void Push(RootParam param, D3D12_ROOT_PARAMETER_TYPE paramType, UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visibility, UINT num32BitValues = 0);
	// add root parameter with descriptor table and range
	void PushTable(RootParam param, D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT shaderRegister, UINT registerSpace, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility);

protected:
	// ID3D12Device::CreateRootSignature
	virtual void Create() abstract;

	// [mPrams]의 인덱스를 매핑한다.
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
