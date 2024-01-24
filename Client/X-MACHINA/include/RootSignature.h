#pragma once

// util of ID3D12RootSignature
// root parameter�� �����Ͽ� �����ϰ� ����ϵ��� �ϰ� ������ �ּ�ȭ�Ѵ�.
class GraphicsRootSignature {
private:
	ComPtr<ID3D12RootSignature> mRootSignature{};

	std::vector<D3D12_ROOT_PARAMETER>	mParams{};		// all root parameters
	std::vector<D3D12_DESCRIPTOR_RANGE> mRanges{};		// all root ranges
	std::unordered_map<RootParam, UINT> mParamMap{};	// [mPrams]�� �ε��� ��

public:
	GraphicsRootSignature();
	virtual ~GraphicsRootSignature() = default;

	RComPtr<ID3D12RootSignature> Get() const		{ return mRootSignature; }
	UINT GetRootParamIndex(RootParam param) const	{ return mParamMap.at(param); }

public:
	// ��Ī�� �߰��� �ϳ��� root parameter�� ���� �̸����� ������ �� �ְ� �Ѵ�.
	void AddAlias(RootParam origin, RootParam alias) { mParamMap[alias] = mParamMap[origin]; }

	// add root parameter
	void Push(RootParam param, D3D12_ROOT_PARAMETER_TYPE paramType, UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visibility, UINT num32BitValues = 0);
	// add root parameter with descriptor table and range
	void PushTable(RootParam param, D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT shaderRegister, UINT registerSpace, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility);

	// ID3D12Device::CreateRootSignature
	RComPtr<ID3D12RootSignature> Create();

private:
	// [mPrams]�� �ε����� �����Ѵ�.
	void ParamMapping(RootParam param);
};