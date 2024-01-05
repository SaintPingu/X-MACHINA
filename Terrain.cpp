#include "stdafx.h"
#include "Terrain.h"
#include "DXGIMgr.h"

#include "Model.h"
#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"
#include "Scene.h"
#include "Texture.h"

#define _WITH_APPROXIMATE_OPPOSITE_CORNER

namespace {
	float uint16ToFloat(std::uint16_t value)
	{
		return static_cast<float>(value) / 65535.0f * 255.0f;
	}

	// �߽���(center)�� �������� �������� ��ġ�� ��ȸ�ϸ� BoundingSphere�� ���
	MyBoundingSphere CalculateBoundingSphere(const Vec3& center, const std::vector<Vec3>& positions)
	{
		float maxDistanceSq = 0.f;

		// ��� ���� ��ġ�� ���� ��� �߽ɰ� �ִ� �Ÿ� ���
		for (const auto& position : positions) {
			float distanceSq = (position.x - center.x) * (position.x - center.x) +
				(position.y - center.y) * (position.y - center.y) +
				(position.z - center.z) * (position.z - center.z);
			maxDistanceSq = max(maxDistanceSq, distanceSq);
		}

		// BoundingSphere ������ ����
		float radius = std::sqrt(maxDistanceSq);

		MyBoundingSphere result{};
		result.SetOrigin(center);
		result.Center = center;
		result.Radius = radius;
		return result;
	}

	// �������� ��ġ�� ��ȸ�ϸ� BoundingSphere�� ���
	MyBoundingSphere CalculateBoundingSphere(const std::vector<Vec3>& positions)
	{
		Vec3 center{ 0.f, 0.f, 0.f };
		float maxDistanceSq = 0.f;

		// ��� ���� ��ġ�� ���� ��� �߽� ��ġ ���
		for (const auto& position : positions) {
			center.x += position.x;
			center.y += position.y;
			center.z += position.z;
		}
		int vertexCount = positions.size();
		center.x /= vertexCount;
		center.y /= vertexCount;
		center.z /= vertexCount;

		return CalculateBoundingSphere(center, positions);
	}
}





#pragma region HeightMapImage
HeightMapImage::HeightMapImage(const std::wstring& fileName, int width, int length)
	:
	mWidth(width),
	mLength(length)	// [ERROR] '*2' ���ϸ� ���ݹۿ� �ε��� �ȵǴ� ����
{
	std::vector<uint16_t> pHeightMapPixels(mWidth * length * 2);

	//������ ���� �д´�. ���� �� �̹����� ���� ����� ���� RAW �̹����̴�.
	HANDLE hFile = ::CreateFile(fileName.data(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, nullptr);

	DWORD dwBytesRead;
	BOOL success = ::ReadFile(hFile, pHeightMapPixels.data(), (mWidth * length * 2), &dwBytesRead, nullptr);
	::CloseHandle(hFile);

	mHeightMapPixels.resize(mWidth * length * 2);
	for (int y = 0; y < length * 2; y++)
	{
		for (int x = 0; x < mWidth; x++)
		{
			mHeightMapPixels[x + (y * mWidth)] = uint16ToFloat(pHeightMapPixels[x + (y * mWidth)]);
		}
	}
}


float HeightMapImage::GetHeight(float fx, float fz) const
{
	/*������ ��ǥ (fx, fz)�� �̹��� ��ǥ���̴�. ���� ���� x-��ǥ�� z-��ǥ�� ���� ���� ������ ����� ������ ���̴� 0�̴�.*/
	if ((fx < 0.f) || (fz < 0.f) || (fx >= mWidth - 1) || (fz >= mLength - 1)) {
		return 0.f;
	}
	//���� ���� ��ǥ�� ���� �κа� �Ҽ� �κ��� ����Ѵ�.
	const int x          = static_cast<int>(fx);
	const int z          = static_cast<int>(fz);
	const float xPercent = fx - x;
	const float zPercent = fz - z;

	float bottomLeft  = static_cast<float>(mHeightMapPixels[x + (z * mWidth)]);
	float bottomRight = static_cast<float>(mHeightMapPixels[(x + 1) + (z * mWidth)]);
	float topLeft     = static_cast<float>(mHeightMapPixels[x + ((z + 1) * mWidth)]);
	float topRight    = static_cast<float>(mHeightMapPixels[(x + 1) + ((z + 1) * mWidth)]);

#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	//z-��ǥ�� 1, 3, 5, ...�� ��� �ε����� �����ʿ��� �������� �����ȴ�.
	const bool isRightToLeft = ((z % 2) != 0);
	if (isRightToLeft)
	{
		/*������ �ﰢ������ �����ʿ��� ���� �������� �����Ǵ� ����̴�.
		���� �׸��� �������� (zPercent < xPercent) �� ����̴�.
		�� ��� TopLeft�� �ȼ� ���� (topLeft = topRight + (bottomLeft - bottomRight))�� �ٻ��Ѵ�.
		���� �׸��� ������ (zPercent �� xPercent)�� ����̴�.
		�� ��� BottomRight�� �ȼ� ���� (bottomRight = bottomLeft + (topRight - topLeft))�� �ٻ��Ѵ�.*/

		if (zPercent >= xPercent)
			bottomRight = bottomLeft + (topRight - topLeft);
		else
			topLeft = topRight + (bottomLeft - bottomRight);
	}
	else
	{
		/*������ �ﰢ������ ���ʿ��� ������ �������� �����Ǵ� ����̴�.
		���� �׸��� ������ (zPercent < (1.f - xPercent))�� ����̴�.
		�� ��� TopRight�� �ȼ� ���� (topRight = topLeft + (bottomRight - bottomLeft))�� �ٻ��Ѵ�.
		���� �׸��� �������� (zPercent �� (1.f - xPercent))�� ����̴�.
		�� ��� BottomLeft�� �ȼ� ���� (bottomLeft = topLeft + (bottomRight - topRight))�� �ٻ��Ѵ�.*/
		if (zPercent < (1.f - xPercent))
			topRight = topLeft + (bottomRight - bottomLeft);
		else
			bottomLeft = topLeft + (bottomRight - topRight);
	}
#endif

	//�簢���� �� ���� �����Ͽ� ����(�ȼ� ��)�� ����Ѵ�.
	float topHeight = topLeft * (1 - xPercent) + topRight * xPercent;
	float bottomHeight = bottomLeft * (1 - xPercent) + bottomRight * xPercent;
	float height = bottomHeight * (1 - zPercent) + topHeight * zPercent;
	return height;
}

Vec3 HeightMapImage::GetHeightMapNormal(int x, int z) const
{
	// x - ��ǥ�� z - ��ǥ�� ���� ���� ������ ����� ������ ���� ���ʹ� y - �� ���� �����̴�.
	if ((x < 0.f) || (z < 0.f) || (x > mWidth) || (z > mLength)) {
		return Vec3(0.f, 1.f, 0.f);
	}

	/*���� �ʿ��� (x, z) ��ǥ�� �ȼ� ���� ������ �� ���� �� (x+1, z), (z, z+1)�� ���� �ȼ� ���� ����Ͽ� ���� ���͸� ����Ѵ�.*/
	const int heightMapIndex = x + (z * mWidth);
	const int xHeightMapAdd  = (x < (mWidth - 1)) ? 1 : -1;
	const int zHeightMapAdd  = (z < (mLength - 1)) ? mWidth : -mWidth;

	const float y1 = (float)mHeightMapPixels[heightMapIndex];
	const float y2 = (float)mHeightMapPixels[heightMapIndex + xHeightMapAdd];
	const float y3 = (float)mHeightMapPixels[heightMapIndex + zHeightMapAdd];

	const Vec3 edge1  = Vec3(0.f, y3 - y1, 1.f);
	const Vec3 edge2  = Vec3(1.f, y2 - y1, 0.f);
	const Vec3 normal = Vector3::CrossProduct(edge1, edge2, true);

	return normal;
}
#pragma endregion





#pragma region Terrain
Terrain::Terrain(const std::wstring& fileName, int width, int length, int blockWidth, int blockLength) : Transform(this),
	mWidth(width), 
	mLength(length)
{
	mHeightMapImage = std::make_shared<HeightMapImage>(fileName, width, length);

	/*���� ��ü�� ���� �޽����� �迭�� ���� ���̴�. blockWidth, blockLength�� ���� �޽� �ϳ��� ����, ���� ũ���̴�. quadsPerBlock, quadsPerBlock�� ���� �޽��� ���� ����� ���� ���� �簢���� �����̴�.*/
	int xQuadsPerBlock = blockWidth - 1;
	int zQuadsPerBlock = blockLength - 1;

	//�������� ���� ����, ���� �������� ���� �޽��� �� ���� �ִ� ���� ��Ÿ����.
	long xBlocks = (mWidth - 1) / xQuadsPerBlock;
	long zBlocks = (mLength - 1) / zQuadsPerBlock;

	mTerrains.resize(xBlocks * zBlocks);
	mBuffer.resize(xBlocks * zBlocks);

	float meshRadius = std::sqrtf((blockWidth * blockWidth) + (blockLength * blockLength)) * 1.1f;
	MyBoundingSphere bs;
	bs.Radius = meshRadius;

	for (int z = 0, zStart = 0; z < zBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < xBlocks; x++)
		{
			xStart = x * (blockWidth - 1);
			zStart = z * (blockLength - 1);

			// ���� �޽��� ������ ����
			int index = x + (z * xBlocks);

			sptr<TerrainGridMesh> mesh = std::make_shared<TerrainGridMesh>(xStart, zStart, blockWidth, blockLength, mHeightMapImage);
			mTerrains[index]           = std::make_shared<TerrainBlock>(mesh, this);

			Vec3 center = Vec3(xStart + blockWidth / 2, 0.f, zStart + blockLength / 2);
			bs.Center   = center;
			mTerrains[index]->SetPosition(center);
			mTerrains[index]->AddComponent<SphereCollider>()->mBS = bs;
		}
	}

	MaterialLoadInfo materialInfo{};
	materialInfo.Emissive          = Vec4(0.f, 0.f, 0.f, 1.f);
	materialInfo.Glossiness        = 0.1414213f;
	materialInfo.Metallic          = 0.f;
	materialInfo.SpecularHighlight = 1.f;
	materialInfo.GlossyReflection  = 1.f;

	sptr<MaterialColors> materialColors = std::make_shared<MaterialColors>(materialInfo);
	mMaterial                           = std::make_shared<Material>();
	mMaterial->SetMaterialColors(materialColors);

	mTextureLayer[0] = scene->GetTexture("GrassUV01");
	mTextureLayer[1] = scene->GetTexture("Detail_Texture_6");
	mTextureLayer[2] = scene->GetTexture("Stone");
	mSplatMap        = scene->GetTexture("Terrain_splatmap");

	mTextureLayer[0]->SetRootParamIndex(scene->GetRootParamIndex(RootParam::TerrainLayer0));
	mTextureLayer[1]->SetRootParamIndex(scene->GetRootParamIndex(RootParam::TerrainLayer1));
	mTextureLayer[2]->SetRootParamIndex(scene->GetRootParamIndex(RootParam::TerrainLayer2));
	mSplatMap->SetRootParamIndex(scene->GetRootParamIndex(RootParam::SplatMap));

	mShader = std::make_shared<TerrainShader>();
	mShader->Create();
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

	UpdateShaderVars();
	mMaterial->UpdateShaderVars();

	mTextureLayer[0]->UpdateShaderVars();
	mTextureLayer[1]->UpdateShaderVars();
	mTextureLayer[2]->UpdateShaderVars();
	mSplatMap->UpdateShaderVars();

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
	static constexpr float kCorr = 1.f / gkTerrainLength;	// for SplatMap
	static constexpr float kDetailScale = 0.1f;

	mVertexCnt = width * length;
	std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	std::vector<Vec2> uvs0;
	std::vector<Vec2> uvs1;
	positions.resize(width * length);
	normals.resize(width * length);
	uvs0.resize(width * length);
	uvs1.resize(width * length);

	float height = 0.f;
	float minHeight = FLT_MAX;
	float maxHeight = FLT_MIN;

	for (int i = 0, z = zStart; z < (zStart + length); z++) {
		for (int x = xStart; x < (xStart + width); x++, i++) {

			positions[i] = Vec3(x, OnGetHeight(x, z, heightMapImage), z);
			normals[i] = heightMapImage->GetHeightMapNormal(x, z);

			uvs0[i] = Vec2(float(x) * kDetailScale, float(z) * kDetailScale);
			uvs1[i] = Vec2(float(x) * kCorr, float(z) * kCorr);

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

	CreateIndexBuffer(indices);
}


float TerrainGridMesh::OnGetHeight(int x, int z, rsptr<HeightMapImage> heightMapImage)
{
	const std::vector<float>& heightMapPixels = heightMapImage->GetHeightMapPixels();
	const int width = heightMapImage->GetHeightMapWidth();

	return heightMapPixels[x + (z * width)];
}

void TerrainGridMesh::Render() const
{
	cmdList->IASetVertexBuffers(mSlot, mVertexBufferViews.size(), mVertexBufferViews.data());

	if (mIndexBuffer) {
		cmdList->IASetIndexBuffer(&mIndexBufferView);
		cmdList->DrawIndexedInstanced(mIndexCnt, 1, 0, 0, 0);
	}
	else {
		cmdList->DrawInstanced(mVertexCnt, 1, mOffset, 0);
	}
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