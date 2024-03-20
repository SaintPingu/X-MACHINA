#pragma once

#pragma region ClassForwardDecl
class Texture;
struct PassConstants;
#pragma endregion


#pragma region Struct
struct RenderTarget {
	sptr<Texture>				Target{};
	std::array<float, 4>		ClearColor{ 0.f, 0.f, 0.f, 0.f };
};
#pragma endregion

#pragma region Class
class MultipleRenderTarget
{
public:
	static constexpr UINT mMaxRtCnt = 8;

private:
	GroupType mGroupType{};

	D3D12_VIEWPORT	mViewport{};
	D3D12_RECT		mScissorRect{};

	UINT mRtCnt{};
	std::vector<RenderTarget> mRts{};

	ComPtr<ID3D12DescriptorHeap>	mRtvHeap{};							// RTV ��
	sptr<Texture>					mTextureDs{};						// ���� ���ٽ� �ؽ�ó
	D3D12_CPU_DESCRIPTOR_HANDLE		mRtvHeapBegin{};					// RTV �ڵ�(���� �ּ�)

	std::array<D3D12_RESOURCE_BARRIER, mMaxRtCnt> mTargetToResource{};	// ���� Ÿ�ٿ��� ���ҽ���
	std::array<D3D12_RESOURCE_BARRIER, mMaxRtCnt> mResourceToTarget{};	// ���ҽ����� ���� Ÿ������

public:
#pragma region C/Dtor
	MultipleRenderTarget()			= default;
	virtual ~MultipleRenderTarget() = default;
#pragma endregion

public:
#pragma region Getter
	template<typename T>
	inline rsptr<Texture> GetTexture(T index) const
	{
		return mRts[static_cast<UINT8>(index)].Target;
	}
#pragma endregion

public:
	void Create(GroupType groupType, std::vector<RenderTarget>&& rts, sptr<Texture> dsvHandle, Vec4 clearValue = Vec4{ 0.f });

	// set render targets
	void OMSetRenderTargets();
	void OMSetRenderTargets(UINT count, UINT index);

	// clear RTVs
	void ClearRenderTargetView();
	void ClearRenderTargetView(UINT index);

	// resource barrier for only render target
	void WaitTargetToResource();
	void WaitResourceToTarget();
	void WaitTargetToResource(UINT index);
	void WaitResourceToTarget(UINT index);

	// clear render targets vec
	void ReleaseRenderTargets();

};
#pragma endregion

