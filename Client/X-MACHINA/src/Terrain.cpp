#include "stdafx.h"
#include "Terrain.h"
#include "FrameResource.h"

#include "Model.h"
#include "Shader.h"
#include "Mesh.h"
#include "Scene.h"
#include "Texture.h"
#include "Collider.h"

#define _WITH_APPROXIMATE_OPPOSITE_CORNER

namespace {
	// 템플릿 함수에서 사용하기 위해 uint16ToFloat 함수를 stdafx.h => Math 네임 스페이스로 이동

	// 중심점(center)을 기준으로 정점들의 위치를 순회하며 BoundingSphere를 계산
	MyBoundingSphere CalculateBoundingSphere(const Vec3& center, const std::vector<Vec3>& positions)
	{
		float maxDistanceSq = 0.f;

		// 모든 정점 위치에 대해 평균 중심간 최대 거리 계산
		for (const auto& position : positions) {
			float distanceSq = (position.x - center.x) * (position.x - center.x) +
				(position.y - center.y) * (position.y - center.y) +
				(position.z - center.z) * (position.z - center.z);
			maxDistanceSq = max(maxDistanceSq, distanceSq);
		}

		// BoundingSphere 반지름 설정
		const float radius = std::sqrt(maxDistanceSq);

		MyBoundingSphere result{};
		result.SetOrigin(center);
		result.Center = center;
		result.Radius = radius;
		return result;
	}

	// 정점들의 위치를 순회하며 BoundingSphere를 계산
	MyBoundingSphere CalculateBoundingSphere(const std::vector<Vec3>& positions)
	{
		Vec3 center{ 0.f, 0.f, 0.f };

		// 모든 정점 위치에 대한 평균 중심 위치 계산
		for (const auto& position : positions) {
			center.x += position.x;
			center.y += position.y;
			center.z += position.z;
		}
		const size_t vertexCount = positions.size();
		center.x /= vertexCount;
		center.y /= vertexCount;
		center.z /= vertexCount;

		return CalculateBoundingSphere(center, positions);
	}
}





#pragma region HeightMapImage
HeightMapImage::HeightMapImage(const std::wstring& fileName)
{
#pragma region Extract
	// HeightMap name example : "HeightMap_513x513_R16.raw"
	size_t underScorePos = fileName.find(L"HeightMap_") + std::wstring{ L"HeightMap_" }.length() - 1;
	size_t fileExtPos = fileName.find(std::filesystem::path(fileName).extension());

	// desiredPart name example : "513x513_R16"
	std::wstring desiredPart = (underScorePos != std::wstring::npos && fileExtPos != std::wstring::npos)
		? fileName.substr(underScorePos + 1, fileExtPos - underScorePos - 1) : L"";

	// fileFormat name example : "R16", "R32"
	underScorePos = desiredPart.find(L"_");
	std::wstring fileFormat = desiredPart.substr(underScorePos + 1);

	// extract width, length
	size_t xPos = desiredPart.find(L"x");
	mWidth = std::stoi(desiredPart.substr(0, xPos));
	mLength = std::stoi(desiredPart.substr(xPos + 1));
#pragma endregion

#pragma region Load
	switch (Hash(fileFormat))
	{
	case Hash("R16"):
		LoadHeightMap<uint16_t>(fileName);
		break;
	case Hash("R32"):
		LoadHeightMap<float>(fileName);
		break;
	default:
		break;
	}
#pragma endregion
}


float HeightMapImage::GetHeight(float fx, float fz) const
{
	/*지형의 좌표 (fx, fz)는 이미지 좌표계이다. 높이 맵의 x-좌표와 z-좌표가 높이 맵의 범위를 벗어나면 지형의 높이는 0이다.*/
	if ((fx < 0.f) || (fz < 0.f) || (fx >= mWidth - 1) || (fz >= mLength - 1)) {
		return 0.f;
	}
	//높이 맵의 좌표의 정수 부분과 소수 부분을 계산한다.
	const size_t x       = (size_t)fx;
	const size_t z       = (size_t)fz;
	const float xPercent = fx - x;
	const float zPercent = fz - z;

	float bottomLeft  = (float)mHeightMapPixels[x       + (z * mWidth)];
	float bottomRight = (float)mHeightMapPixels[(x + 1) + (z * mWidth)];
	float topLeft     = (float)mHeightMapPixels[x       + ((z + 1) * mWidth)];
	float topRight    = (float)mHeightMapPixels[(x + 1) + ((z + 1) * mWidth)];

#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	//z-좌표가 1, 3, 5, ...인 경우 인덱스가 오른쪽에서 왼쪽으로 나열된다.
	const bool isRightToLeft = ((z % 2) != 0);
	if (isRightToLeft)
	{
		/*지형의 삼각형들이 오른쪽에서 왼쪽 방향으로 나열되는 경우이다.
		다음 그림의 오른쪽은 (zPercent < xPercent) 인 경우이다.
		이 경우 TopLeft의 픽셀 값은 (topLeft = topRight + (bottomLeft - bottomRight))로 근사한다.
		다음 그림의 왼쪽은 (zPercent ≥ xPercent)인 경우이다.
		이 경우 BottomRight의 픽셀 값은 (bottomRight = bottomLeft + (topRight - topLeft))로 근사한다.*/

		if (zPercent >= xPercent)
			bottomRight = bottomLeft + (topRight - topLeft);
		else
			topLeft = topRight + (bottomLeft - bottomRight);
	}
	else
	{
		/*지형의 삼각형들이 왼쪽에서 오른쪽 방향으로 나열되는 경우이다.
		다음 그림의 왼쪽은 (zPercent < (1.f - xPercent))인 경우이다.
		이 경우 TopRight의 픽셀 값은 (topRight = topLeft + (bottomRight - bottomLeft))로 근사한다.
		다음 그림의 오른쪽은 (zPercent ≥ (1.f - xPercent))인 경우이다.
		이 경우 BottomLeft의 픽셀 값은 (bottomLeft = topLeft + (bottomRight - topRight))로 근사한다.*/
		if (zPercent < (1.f - xPercent))
			topRight = topLeft + (bottomRight - bottomLeft);
		else
			bottomLeft = topLeft + (bottomRight - topRight);
	}
#endif

	//사각형의 네 점을 보간하여 높이(픽셀 값)를 계산한다.
	const float topHeight    = topLeft * (1 - xPercent) + topRight * xPercent;
	const float bottomHeight = bottomLeft * (1 - xPercent) + bottomRight * xPercent;
	const float height       = bottomHeight * (1 - zPercent) + topHeight * zPercent;
	return height;
}

Vec3 HeightMapImage::GetHeightMapNormal(int x, int z) const
{
	// x - 좌표와 z - 좌표가 높이 맵의 범위를 벗어나면 지형의 법선 벡터는 y - 축 방향 벡터이다.
	if ((x < 0.f) || (z < 0.f) || (x > mWidth) || (z > mLength)) {
		return Vec3(0.f, 1.f, 0.f);
	}

	/*높이 맵에서 (x, z) 좌표의 픽셀 값과 인접한 두 개의 점 (x+1, z), (z, z+1)에 대한 픽셀 값을 사용하여 법선 벡터를 계산한다.*/
	const int heightMapIndex = x + (z * mWidth);
	const int xHeightMapAdd  = (x < (mWidth - 1))  ? 1 : -1;
	const int zHeightMapAdd  = (z < (mLength - 1)) ? mWidth : -mWidth;

	const float y1 = (float)mHeightMapPixels[(size_t)heightMapIndex];
	const float y2 = (float)mHeightMapPixels[(size_t)heightMapIndex + xHeightMapAdd];
	const float y3 = (float)mHeightMapPixels[(size_t)heightMapIndex + zHeightMapAdd];

	const Vec3 edge1  = Vec3(0.f, y3 - y1, 1.f);
	const Vec3 edge2  = Vec3(1.f, y2 - y1, 0.f);
	const Vec3 normal = Vector3::CrossProduct(edge1, edge2, true);

	return normal;
}
#pragma endregion





#pragma region Terrain
Terrain::Terrain(const std::wstring& fileName) : Transform(this)
{
	mHeightMapImage = std::make_shared<HeightMapImage>(fileName);

	mWidth = mHeightMapImage->GetHeightMapWidth();
	mLength = mHeightMapImage->GetHeightMapLength();

	int blockWidth = (mWidth - 1) / 8 + 1;
	int blockLength = (mLength - 1) / 8 + 1;

	/*지형 객체는 격자 메쉬들의 배열로 만들 것이다. blockWidth, blockLength는 격자 메쉬 하나의 가로, 세로 크기이다. quadsPerBlock, quadsPerBlock은 격자 메쉬의 가로 방향과 세로 방향 사각형의 개수이다.*/
	const int xQuadsPerBlock = blockWidth - 1;
	const int zQuadsPerBlock = blockLength - 1;

	//지형에서 가로 방향, 세로 방향으로 격자 메쉬가 몇 개가 있는 가를 나타낸다.
	const size_t xBlocks = (size_t)(mWidth - 1) / xQuadsPerBlock;
	const size_t zBlocks = (size_t)(mLength - 1) / zQuadsPerBlock;

	mTerrains.resize(xBlocks * zBlocks);
	mBuffer.resize(xBlocks * zBlocks);

	// 메쉬 Bounding Sphere 반지름 = 대각선 길이 * n(보정값)
	MyBoundingSphere bs;
	bs.Radius = std::sqrtf((float)(blockWidth * blockWidth) + (blockLength * blockLength)) * 1.1f;

	// 격자마다 메쉬를 생성해 TerrainBlock에 저장
	for (int z = 0, zStart = 0; z < zBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < xBlocks; x++)
		{
			xStart             = x * (blockWidth - 1);
			zStart             = z * (blockLength - 1);
			const size_t index = x + (xBlocks * z);

			const sptr<TerrainGridMesh> mesh = std::make_shared<TerrainGridMesh>(xStart, zStart, blockWidth, blockLength, mHeightMapImage);
			mTerrains[index]                 = std::make_shared<TerrainBlock>(mesh, this);

			const Vec3 center = Vec3(xStart + (float)blockWidth / 2, 0.f, zStart + (float)blockLength / 2);
			bs.Center   = center;
			mTerrains[index]->SetPosition(center);
			mTerrains[index]->AddComponent<SphereCollider>()->mBS = bs;
		}
	}

	MaterialLoadInfo materialInfo{};
	materialInfo.DiffuseAlbedo	= Vec4(1.f, 1.f, 1.f, 1.f);
	materialInfo.Metallic		= 0.01f;
	materialInfo.Roughness		= 0.95f;

	sptr<MaterialColors> materialColors = std::make_shared<MaterialColors>(materialInfo);
	mMaterial                           = std::make_shared<Material>();
	mMaterial->SetMaterialColors(materialColors);

	mMaterial->SetTexture(TextureMap::DiffuseMap0, scene->GetTexture("GrassUV01"));
	mMaterial->SetTexture(TextureMap::DiffuseMap1, scene->GetTexture("Detail_Texture_6"));
	mMaterial->SetTexture(TextureMap::DiffuseMap2, scene->GetTexture("Stone"));
	mMaterial->SetTexture(TextureMap::DiffuseMap3, scene->GetTexture("Terrain_splatmap"));

	mShader = std::make_shared<TerrainShader>();
	mShader->Create(ShaderType::Deferred);
}



float Terrain::GetHeight(float x, float z) const
{
	return mHeightMapImage->GetHeight(x, z);
}

Vec3 Terrain::GetNormal(float x, float z) const
{
	return mHeightMapImage->GetHeightMapNormal(static_cast<int>(x), static_cast<int>(z));
}

int Terrain::GetHeightMapWidth() const
{
	return mHeightMapImage->GetHeightMapWidth();
}
int Terrain::GetHeightMapLength() const
{
	return mHeightMapImage->GetHeightMapLength();
}

void Terrain::PushObject(TerrainBlock* block)
{
	assert(mCurrBuffIdx < mBuffer.size());

	mBuffer[mCurrBuffIdx++] = block;
}

void Terrain::Render()
{
	mShader->Set();

	mMaterial->UpdateShaderVars();
	UpdateShaderVars(0, mMaterial->mMatIndex);

	for (UINT i = 0; i < mCurrBuffIdx; ++i) {
		mBuffer[i]->RenderMesh();
	}

	ResetBuffer();
}

void Terrain::OnEnable()
{
	for (auto& terrain : mTerrains) {
		terrain->OnEnable();
	}
}

void Terrain::Awake()
{
	for (auto& terrain : mTerrains) {
		terrain->Awake();
	}
}


void Terrain::UpdateGrid()
{
	for (auto& terrain : mTerrains) {
		scene->UpdateObjectGrid(terrain.get());
	}
}
#pragma endregion





#pragma region TerrainGridMesh
TerrainGridMesh::TerrainGridMesh(int xStart, int zStart, int width, int length, rsptr<HeightMapImage> heightMapImage)
{
	const float kCorr = 1.f / heightMapImage->GetHeightMapLength();	// for SplatMap
	const float kDetailScale = 0.1f;

	mVertexCnt = width * length;
	std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	std::vector<Vec2> uvs0;
	std::vector<Vec2> uvs1;
	positions.resize((size_t)width * length);
	normals.resize((size_t)width * length);
	uvs0.resize((size_t)width * length);
	uvs1.resize((size_t)width * length);

	float height = 0.f;
	float minHeight = FLT_MAX;
	float maxHeight = FLT_MIN;

	for (int i = 0, z = zStart; z < (zStart + length); z++) {
		for (int x = xStart; x < (xStart + width); x++, i++) {

			positions[i] = Vec3((float)x, OnGetHeight(x, z, heightMapImage), (float)z);
			normals[i] = heightMapImage->GetHeightMapNormal(x, z);

			uvs0[i] = Vec2((float)x * kDetailScale, (float)z * kDetailScale);
			uvs1[i] = Vec2((float)x * kCorr,		(float)z * kCorr);

			if (height < minHeight) {
				minHeight = height;
			}
			if (height > maxHeight) {
				maxHeight = height;
			}
		}
	}

	D3DUtil::CreateVertexBufferResource(positions, mVertexUploadBuffer, mVertexBuffer);
	D3DUtil::CreateVertexBufferResource(normals, mNormalUploadBuffer, mNormalBuffer);
	D3DUtil::CreateVertexBufferResource(uvs0, mUV0UploadBuffer, mUV0Buffer);
	D3DUtil::CreateVertexBufferResource(uvs1, mUV1UploadBuffer, mUV1Buffer);

	CreateVertexBufferViews();


	mIndexCnt = ((width * 2) * (length - 1)) + ((length - 1) - 1);
	std::vector<UINT> indices(mIndexCnt);
	for (int j = 0, z = 0; z < length - 1; z++) {
		if ((z % 2) == 0) {
			for (int x = 0; x < width; x++) {
				if ((x == 0) && (z > 0)) {
					indices[j++] = (UINT)(x + (z * width));
				}
				indices[j++] = (UINT)(x + (z * width));
				indices[j++] = (UINT)((x + (z * width)) + width);
			}
		}
		else {
			for (int x = width - 1; x >= 0; x--) {
				if (x == (width - 1)) {
					indices[j++] = (UINT)(x + (z * width));
				}
				indices[j++] = (UINT)(x + (z * width));
				indices[j++] = (UINT)((x + (z * width)) + width);
			}
		}
	}

	CreateIndexBufferView(indices);
}


float TerrainGridMesh::OnGetHeight(int x, int z, rsptr<HeightMapImage> heightMapImage)
{
	const std::vector<float>& heightMapPixels = heightMapImage->GetHeightMapPixels();
	const int width = heightMapImage->GetHeightMapWidth();

	return heightMapPixels[(size_t)x + ((size_t)z * width)];
}
#pragma endregion





#pragma region TerrainBlock
TerrainBlock::TerrainBlock(rsptr<TerrainGridMesh> mesh, Terrain* terrain)
	:
	mMesh(mesh),
	mBuffer(terrain)
{
	SetTag(ObjectTag::Terrain);
}

void TerrainBlock::RenderMesh()
{
	mMesh->Render();
	Update();
}

void TerrainBlock::Push()
{
	if (mIsPushed) {
		return;
	}

	mIsPushed = true;
	mBuffer->PushObject(this);
}
#pragma endregion