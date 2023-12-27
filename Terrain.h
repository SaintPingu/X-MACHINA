#pragma once
#include "Object.h"
#include "Mesh.h"

constexpr int TERRAIN_LENGTH = 513;

class Material;
class Texture;
class HeightMapTerrain;
class IlluminatedShader;
class TerrainShader;

class HeightMapImage {
private:
	std::vector<float> mHeightMapPixels;

	int mWidth;
	int mLength;

public:
	HeightMapImage(LPCTSTR fileName, int width, int length);
	~HeightMapImage(void);

	float GetHeight(float fx, float fz);
	Vec3 GetHeightMapNormal(int x, int z);
	const std::vector<float>& GetHeightMapPixels() { return(mHeightMapPixels); }
	int GetHeightMapWidth() { return(mWidth); }
	int GetHeightMapLength() { return(mLength); }
};


class HeightMapGridMesh : public Mesh {
public:
	HeightMapGridMesh(int xStart, int zStart, int width, int length, rsptr<HeightMapImage> heightMapImage);
	virtual ~HeightMapGridMesh();

	virtual float OnGetHeight(int x, int z, rsptr<HeightMapImage> heightMapImage);
	void Render() const;
};


class TerrainBlock;
class HeightMapTerrain : public Transform {
public:
	HeightMapTerrain(LPCTSTR fileName, int width, int length, int blockWidth, int blockLength);
	virtual ~HeightMapTerrain();
private:
	sptr<HeightMapImage> mHeightMapImage{};

	std::vector<sptr<TerrainBlock>> mTerrains;
	std::vector<TerrainBlock*> mBuffer;
	UINT mCrntBufferIndex{};

	sptr<TerrainShader> mShader;

	sptr<Material> mMaterial{};
	sptr<Texture> mTextureLayer[4];
	sptr<Texture> mSplatMap{};

	int mWidth{};
	int mLength{};

	void ResetBuffer() { mCrntBufferIndex = 0; }
public:
	float GetHeight(float x, float z);

	Vec3 GetNormal(float x, float z);

	int GetHeightMapWidth();
	int GetHeightMapLength();

	int GetWidth() { return mWidth; }
	int GetLength() { return mLength; }

	const std::vector<sptr<TerrainBlock>>& GetTerrains() const { return mTerrains; }

	void PushObject(TerrainBlock* terrain);
	void Render();

	void Start();
	virtual void Update() override;
};


class TerrainBlock : public GameObject {
private:
	HeightMapTerrain* mBuffer{};
	sptr<HeightMapGridMesh> mMesh;

	bool mIsPushed{ false };
	void Reset() { mIsPushed = false; }
	void Push();

public:
	TerrainBlock(rsptr<HeightMapGridMesh> mesh);
	void SetTerrain(HeightMapTerrain* terrain) { mBuffer = terrain; }

	virtual void Update() override { Reset(); };
	virtual void Render() override;
	void RenderMesh();
};