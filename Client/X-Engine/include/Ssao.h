#pragma once

#pragma region ClassForwardDecl
class Texture;
#pragma endregion

#pragma region Class
class Ssao : private UnCopyable {
private:
	Vec4 mOffsets[14];
	ComPtr<ID3D12Resource> mRandomVecMap;
	ComPtr<ID3D12Resource> mRandomVecMapUploadBuffer;

	sptr<Texture> mSsao0Map;
	sptr<Texture> mSsao1Map;

public:
#pragma region C/Dtor
	Ssao(ID3D12GraphicsCommandList* directCmdList);
	virtual ~Ssao() = default;
#pragma endregion

public:
	void GetOffsetVectors(Vec4 offsets[14]);
	void Execute(int blurCount);
	void RenderSsaoMap();
	void BlurSsaoMap(int blurCount);
	void BlurSsaoMap(bool horzBlur);

private:
	void CreateRandomVectorTexture(ID3D12GraphicsCommandList* directCmdList);
	void CreateOffsetVectors();
};
#pragma endregion