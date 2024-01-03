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
// 2D height map 정보를 읽고 저장한다.
class HeightMapImage {
private:
	std::vector<float> mHeightMapPixels{};

	int mWidth{};
	int mLength{};

public:
	// [fileName]의 파일을 [width * height]만큼 읽어 저장한다.
	HeightMapImage(const std::wstring& fileName, int width, int length);
	virtual ~HeightMapImage() = default;

	float GetHeight(float x, float z) const;
	Vec3 GetHeightMapNormal(int x, int z) const;

	const std::vector<float>& GetHeightMapPixels() const { return mHeightMapPixels; }
	int GetHeightMapWidth() const { return mWidth; }
	int GetHeightMapLength() const { return mLength; }
};





// 전체 Terrain 관리
class Terrain : public Transform {
private:
	sptr<HeightMapImage> mHeightMapImage{};

	std::vector<sptr<TerrainBlock>> mTerrains{};	// all Blocks
	std::vector<TerrainBlock*>		mBuffer{};		// 카메라에 보이는 Blocks (rendering buffer)
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





// grid로 분할된 mesh
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
	// grid의 좌표(x, z)에 대한 정점의 높이를 반환한다.
	virtual float OnGetHeight(int x, int z, rsptr<HeightMapImage> heightMapImage);
};





// grid로 분할된 각 Terrain의 일부 Block
// 렌더링 시 Terrain 전체를 렌더링하는 것이 아닌 카메라에 보이는 부분만 렌더링 하기 위함이다.
class TerrainBlock : public GameObject {
private:
	sptr<TerrainGridMesh>	mMesh{};	// Block마다 mesh를 보유한다.
	Terrain*				mBuffer{};	// Terrain을 버퍼로 한다. Scene의 Grid에 속하며, 카메라에 보이면 Terrain의 버퍼에 추가된다.

	bool mIsPushed{ false };			// rendering buffer에 추가되었는가?

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
	// 이 Block을 rendering buffer에 추가한다.
	void Push();
};
#pragma endregion
