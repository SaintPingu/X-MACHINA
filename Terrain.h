#pragma once


#pragma region Include
#include "Object.h"
#include "Mesh.h"
#pragma endregion


#pragma region ClassForwardDecl
class Material;
class Texture;
class HeightMapTerrain;
class IlluminatedShader;
class TerrainShader;
class TerrainBlock;
#pragma endregion


#pragma region Variable
constexpr int TERRAIN_LENGTH = 513;
#pragma endregion


#pragma region Class
class HeightMapImage {
private:
	std::vector<float> mHeightMapPixels{};

	int mWidth{};
	int mLength{};

public:
	HeightMapImage(const std::wstring& fileName, int width, int length);
	virtual ~HeightMapImage() = default;

	float GetHeight(float fx, float fz) const;
	Vec3 GetHeightMapNormal(int x, int z) const;

	const std::vector<float>& GetHeightMapPixels() const { return mHeightMapPixels; }
	int GetHeightMapWidth() const { return mWidth; }
	int GetHeightMapLength() const { return mLength; }
};





class HeightMapGridMesh : public Mesh {
public:
	HeightMapGridMesh(int xStart, int zStart, int width, int length, rsptr<HeightMapImage> heightMapImage);
	virtual ~HeightMapGridMesh() = default;

public:
	virtual float OnGetHeight(int x, int z, rsptr<HeightMapImage> heightMapImage);
	void Render() const;
};





class HeightMapTerrain : public Transform {
private:
	sptr<HeightMapImage> mHeightMapImage{};

	std::vector<sptr<TerrainBlock>> mTerrains{};
	std::vector<TerrainBlock*> mBuffer{};
	UINT mCurrBuffIdx{};

	sptr<TerrainShader> mShader{};

	sptr<Material> mMaterial{};
	sptr<Texture> mTextureLayer[4];
	sptr<Texture> mSplatMap{};

	int mWidth{};
	int mLength{};

public:
	HeightMapTerrain(const std::wstring& fileName, int width, int length, int blockWidth, int blockLength);
	virtual ~HeightMapTerrain() = default;

	int GetWidth() const { return mWidth; }
	int GetLength() const { return mLength; }
	int GetHeightMapWidth() const;
	int GetHeightMapLength() const;

	float GetHeight(float x, float z) const;
	Vec3 GetNormal(float x, float z) const;

public:
	void Start();
	virtual void Update() override;
	void UpdateGrid();
	void Render();

	void PushObject(TerrainBlock* terrain);

private:
	void ResetBuffer() { mCurrBuffIdx = 0; }
};





class TerrainBlock : public GameObject {
private:
	HeightMapTerrain* mBuffer{};
	sptr<HeightMapGridMesh> mMesh{};

	bool mIsPushed{ false };

public:
	TerrainBlock(rsptr<HeightMapGridMesh> mesh);
	virtual ~TerrainBlock() = default;

	void SetTerrain(HeightMapTerrain* terrain) { mBuffer = terrain; }

public:
	virtual void Update() override { Reset(); };
	virtual void Render() override;
	void RenderMesh();

private:
	void Reset() { mIsPushed = false; }
	void Push();
};
#pragma endregion
