#pragma once
#include "Object.h"
#include "Mesh.h"

constexpr int TERRAIN_LENGTH = 513;

class CMaterial;
class CTexture;
class CHeightMapTerrain;
class CIlluminatedShader;
class CTerrainShader;

class CHeightMapImage {
private:
	std::vector<float> mHeightMapPixels;

	int mWidth;
	int mLength;

public:
	CHeightMapImage(LPCTSTR fileName, int width, int length);
	~CHeightMapImage(void);

	float GetHeight(float fx, float fz);
	Vec3 GetHeightMapNormal(int x, int z);
	const std::vector<float>& GetHeightMapPixels() { return(mHeightMapPixels); }
	int GetHeightMapWidth() { return(mWidth); }
	int GetHeightMapLength() { return(mLength); }
};


class CHeightMapGridMesh : public CMesh {
public:
	CHeightMapGridMesh(int xStart, int zStart, int width, int length, rsptr<CHeightMapImage> heightMapImage);
	virtual ~CHeightMapGridMesh();

	virtual float OnGetHeight(int x, int z, rsptr<CHeightMapImage> heightMapImage);
	void Render() const;
};


class CTerrainBlock;
class CHeightMapTerrain : public Transform {
public:
	CHeightMapTerrain(LPCTSTR fileName, int width, int length, int blockWidth, int blockLength);
	virtual ~CHeightMapTerrain();
private:
	sptr<CHeightMapImage> mHeightMapImage{};

	std::vector<sptr<CTerrainBlock>> mTerrains;
	std::vector<CTerrainBlock*> mBuffer;
	UINT mCrntBufferIndex{};

	sptr<CTerrainShader> mShader;

	sptr<CMaterial> mMaterial{};
	sptr<CTexture> mTextureLayer[4];
	sptr<CTexture> mSplatMap{};

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

	const std::vector<sptr<CTerrainBlock>>& GetTerrains() const { return mTerrains; }

	void PushObject(CTerrainBlock* terrain);
	void Render(bool isMirror = false);

	void Start();
	virtual void Update() override;
};


class CTerrainBlock : public CGameObject {
private:
	CHeightMapTerrain* mBuffer{};
	sptr<CHeightMapGridMesh> mMesh;

	bool mIsPushed{ false };
	void Reset() { mIsPushed = false; }
	void Push();

public:
	CTerrainBlock(rsptr<CHeightMapGridMesh> mesh);
	void SetTerrain(CHeightMapTerrain* terrain) { mBuffer = terrain; }

	virtual void Update() override { Reset(); };
	virtual void Render() override;
	void RenderMesh();
};