#pragma once


#pragma region Include
#include "Object.h"
#include "Mesh.h"
#pragma endregion


#pragma region ClassForwardDecl
class Material;
class Texture;
class Terrain;
class IlluminatedShader;
class TerrainShader;
class TerrainBlock;
#pragma endregion


#pragma region Variable
constexpr int gkTerrainLength = 513;
#pragma endregion


#pragma region Class
// 2D height map ������ �а� �����Ѵ�.
class HeightMapImage {
private:
	std::vector<float> mHeightMapPixels{};

	int mWidth{};
	int mLength{};

public:
	// [fileName]�� ������ [width * height]��ŭ �о� �����Ѵ�.
	HeightMapImage(const std::wstring& fileName, int width, int length);
	virtual ~HeightMapImage() = default;

	float GetHeight(float x, float z) const;
	Vec3 GetHeightMapNormal(int x, int z) const;

	const std::vector<float>& GetHeightMapPixels() const { return mHeightMapPixels; }
	int GetHeightMapWidth() const { return mWidth; }
	int GetHeightMapLength() const { return mLength; }
};





// ��ü Terrain ����
class Terrain : public Transform {
private:
	sptr<HeightMapImage> mHeightMapImage{};

	std::vector<sptr<TerrainBlock>> mTerrains{};	// all Blocks
	std::vector<TerrainBlock*>		mBuffer{};		// ī�޶� ���̴� Blocks (rendering buffer)
	UINT							mCurrBuffIdx{};

	sptr<TerrainShader> mShader{};
	sptr<Material>		mMaterial{};		// default material (apply all textures)
	sptr<Texture>		mTextureLayer[3];	// for texture splatting
	sptr<Texture>		mSplatMap{};

	int mWidth{};	// width  of entire Terrain
	int mLength{};	// length of entire Terrain

public:
	Terrain(const std::wstring& fileName, int width, int length, int blockWidth, int blockLength);
	virtual ~Terrain() = default;

	int GetWidth() const { return mWidth; }
	int GetLength() const { return mLength; }
	int GetHeightMapWidth() const;
	int GetHeightMapLength() const;

	float GetHeight(float x, float z) const;
	Vec3 GetNormal(float x, float z) const;

public:
	void Start();
	// update all terrain block's grid index
	void UpdateGrid();
	// rendering terrain blocks that within [buffer]
	void Render();

	// add terrain [block] to buffer
	void PushObject(TerrainBlock* block);

private:
	void ResetBuffer() { mCurrBuffIdx = 0; }
};





// grid�� ���ҵ� mesh
class TerrainGridMesh : public Mesh {
public:
	// [xStart] : x of grid
	// [zStart] : z of grid
	// [width]  : width  of grid
	// [length] : length of grid
	TerrainGridMesh(int xStart, int zStart, int width, int length, rsptr<HeightMapImage> heightMapImage);
	virtual ~TerrainGridMesh() = default;

public:
	void Render() const;

private:
	// grid�� ��ǥ(x, z)�� ���� ������ ���̸� ��ȯ�Ѵ�.
	virtual float OnGetHeight(int x, int z, rsptr<HeightMapImage> heightMapImage);
};





// grid�� ���ҵ� �� Terrain�� �Ϻ� Block
// ������ �� Terrain ��ü�� �������ϴ� ���� �ƴ� ī�޶� ���̴� �κи� ������ �ϱ� �����̴�.
class TerrainBlock : public GameObject {
private:
	sptr<TerrainGridMesh>	mMesh{};	// Block���� mesh�� �����Ѵ�.
	Terrain*				mBuffer{};	// Terrain�� ���۷� �Ѵ�. Scene�� Grid�� ���ϸ�, ī�޶� ���̸� Terrain�� ���ۿ� �߰��ȴ�.

	bool mIsPushed{ false };			// rendering buffer�� �߰��Ǿ��°�?

public:
	TerrainBlock(rsptr<TerrainGridMesh> mesh, Terrain* terrain);
	virtual ~TerrainBlock() = default;

public:
	virtual void Update() override { Pop(); };
	virtual void Render() override { Push(); }
	// render mesh and Pop()
	void RenderMesh();

private:
	void Pop() { mIsPushed = false; }
	// �� Block�� rendering buffer�� �߰��Ѵ�.
	void Push();
};
#pragma endregion
