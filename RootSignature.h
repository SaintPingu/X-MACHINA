#pragma once

class CGraphicsRootSignature {
private:
	ComPtr<ID3D12RootSignature> mRootSignature{};

	std::vector<D3D12_ROOT_PARAMETER> mParams{};
	std::vector<D3D12_DESCRIPTOR_RANGE> mRanges{};
	std::unordered_map<RootParam, UINT> mParamMap{};

	void ParamMapping(RootParam param);

public:
	CGraphicsRootSignature();
	~CGraphicsRootSignature() = default;

	RComPtr<ID3D12RootSignature> Get() const { return mRootSignature; }
	UINT GetRootParamIndex(RootParam param) { return mParamMap[param]; }

	void AddAlias(RootParam origin, RootParam alias) { mParamMap[alias] = mParamMap[origin]; }

	void Push(RootParam param, D3D12_ROOT_PARAMETER_TYPE paramType, UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility, UINT num32BitValues = 0, UINT registerSpace = 0);
	void PushTable(RootParam param, D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT shaderRegister, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility);

	RComPtr<ID3D12RootSignature> Create();

};