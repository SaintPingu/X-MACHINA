#include "EnginePch.h"
#include "Mesh.h"
#include "DXGIMgr.h"
#include "FrameResource.h"

#include "ResourceMgr.h"
#include "Object.h"
#include "Model.h"
#include "Shader.h"
#include "ObjectPool.h"




#pragma region Mesh
void Mesh::ReleaseUploadBuffers()
{
	mVertexUploadBuffer    = nullptr;
	mNormalUploadBuffer    = nullptr;
	mUV0UploadBuffer       = nullptr;
	mUV1UploadBuffer       = nullptr;
	mTangentUploadBuffer   = nullptr;
	mBiTangentUploadBuffer = nullptr;
	mIndexUploadBuffer     = nullptr;
};

void Mesh::Render() const
{
	CMD_LIST->IASetVertexBuffers(mSlot, (UINT)mVertexBufferViews.size(), mVertexBufferViews.data());

	if (mIndexBuffer) {
		CMD_LIST->IASetIndexBuffer(&mIndexBufferView);
		CMD_LIST->DrawIndexedInstanced(mIndexCnt, 1, 0, 0, 0);
	}
	else {
		CMD_LIST->DrawInstanced(mVertexCnt, 1, mOffset, 0);
	}
}

void Mesh::RenderInstanced(UINT instanceCnt) const
{
	if (instanceCnt <= 0) {
		return;
	}

	CMD_LIST->IASetVertexBuffers(mSlot, (UINT)mVertexBufferViews.size(), mVertexBufferViews.data());

	if (mIndexBuffer) {
		CMD_LIST->IASetIndexBuffer(&mIndexBufferView);
		CMD_LIST->DrawIndexedInstanced(mIndexCnt, instanceCnt, 0, 0, 0);
	}
	else {
		CMD_LIST->DrawInstanced(mVertexCnt, instanceCnt, mOffset, 0);
	}
}

void Mesh::CreateVertexBufferViews()
{
	VertexBufferViews bufferViews{};
	bufferViews.VertexBuffer     = mVertexBuffer;
	bufferViews.NormalBuffer     = mNormalBuffer;
	bufferViews.UV0Buffer        = mUV0Buffer;
	bufferViews.TangentBuffer    = mTangentBuffer;
	bufferViews.BiTangentBuffer  = mBiTangentBuffer;
	bufferViews.BoneIndexBuffer  = mBoneIndexBuffer;
	bufferViews.BoneWeightBuffer = mBoneWeightBuffer;

	D3DUtil::CreateVertexBufferViews(mVertexBufferViews, mVertexCnt, bufferViews);
}

void Mesh::CreateIndexBufferView(const std::vector<UINT>& indices)
{
	D3DUtil::CreateIndexBufferResource(indices, mIndexUploadBuffer, mIndexBuffer);
	D3DUtil::CreateIndexBufferView(mIndexBufferView, mIndexCnt, mIndexBuffer);
}
#pragma endregion





#pragma region ModelObjectMesh
void ModelObjectMesh::CreateMeshFromOBB(const BoundingOrientedBox& box)
{
	std::vector<Vec3> vertices;
	std::vector<UINT> indices;

	mVertexCnt = 8;
	mIndexCnt  = 36;

	vertices.resize(mVertexCnt);

	vertices[0] = box.Center + (box.Extents * Vector3::LDB);
	vertices[1] = box.Center + (box.Extents * Vector3::RDB);
	vertices[2] = box.Center + (box.Extents * Vector3::LUB);
	vertices[3] = box.Center + (box.Extents * Vector3::RDB);
	vertices[4] = box.Center + (box.Extents * Vector3::LDF);
	vertices[5] = box.Center + (box.Extents * Vector3::RDB);
	vertices[6] = box.Center + (box.Extents * Vector3::LUF);
	vertices[7] = box.Center + (box.Extents * Vector3::RUF);

	indices.resize(mIndexCnt);
	indices = {
		0, 1, 2, 2, 1, 3, // Front face
		4, 6, 5, 5, 6, 7, // Back face
		0, 2, 4, 4, 2, 6, // Left face
		1, 5, 3, 3, 5, 7, // Right face
		0, 4, 1, 1, 4, 5, // Bottom face
		2, 3, 6, 6, 3, 7  // Top face
	};


	D3DUtil::CreateVertexBufferResource(vertices, mVertexUploadBuffer, mVertexBuffer);

	CreateVertexBufferViews();
	CreateIndexBufferView(indices);

	
}

void ModelObjectMesh::CreateCubeMesh(float width, float height, float depth, bool hasTexture, bool isLine)
{
	const float x = width * 0.5f, y = height * 0.5f, z = depth * 0.5f;

	std::vector<Vec3> vertices;
	std::vector<UINT> indices;

	if (!isLine) {
		mIndexCnt = 36;
		indices.resize(mIndexCnt);

		if (!hasTexture) {
			mVertexCnt = 8;
			vertices.resize(mVertexCnt);
			vertices[0] = Vec3(-x, +y, -z);
			vertices[1] = Vec3(+x, +y, -z);
			vertices[2] = Vec3(+x, +y, +z);
			vertices[3] = Vec3(-x, +y, +z);
			vertices[4] = Vec3(-x, -y, -z);
			vertices[5] = Vec3(+x, -y, -z);
			vertices[6] = Vec3(+x, -y, +z);
			vertices[7] = Vec3(-x, -y, +z);

			indices = {
				3, 1, 0, 2, 1, 3,
				0, 5, 4, 1, 5, 0,
				3, 4, 7, 0, 4, 3,
				1, 6, 5, 2, 6, 1,
				2, 7, 6, 3, 7, 2,
				6, 4, 5, 7, 4, 6
			};
		}
		else {
			mVertexCnt = 24;
			vertices.resize(mVertexCnt);
			vertices[0] = Vec3(-x, -y, -z);
			vertices[1] = Vec3(-x, +y, -z);
			vertices[2] = Vec3(+x, +y, -z);
			vertices[3] = Vec3(+x, -y, -z);

			vertices[4] = Vec3(-x, -y, +z);
			vertices[5] = Vec3(+x, -y, +z);
			vertices[6] = Vec3(+x, +y, +z);
			vertices[7] = Vec3(-x, +y, +z);

			vertices[8] = Vec3(-x, +y, -z);
			vertices[9] = Vec3(-x, +y, +z);
			vertices[10] = Vec3(+x, +y, +z);
			vertices[11] = Vec3(+x, +y, -z);

			vertices[12] = Vec3(-x, -y, -z);
			vertices[13] = Vec3(+x, -y, -z);
			vertices[14] = Vec3(+x, -y, +z);
			vertices[15] = Vec3(-x, -y, +z);

			vertices[16] = Vec3(-x, -y, +z);
			vertices[17] = Vec3(-x, +y, +z);
			vertices[18] = Vec3(-x, +y, -z);
			vertices[19] = Vec3(-x, -y, -z);

			vertices[20] = Vec3(+x, -y, -z);
			vertices[21] = Vec3(+x, +y, -z);
			vertices[22] = Vec3(+x, +y, +z);
			vertices[23] = Vec3(+x, -y, +z);

			indices = {
				0, 1, 2, 0, 2, 3,
				4, 5, 6, 4, 6, 7,
				8, 9, 10, 8, 10, 11,
				12, 13, 14, 12, 14, 15,
				16, 17, 18, 16, 18, 19,
				20, 21, 22, 20, 22, 23
			};

			std::vector<Vec3> normals(mVertexCnt);
			normals[0] = Vec3(0.f, 0.f, -1.f);
			normals[1] = Vec3(0.f, 0.f, -1.f);
			normals[2] = Vec3(0.f, 0.f, -1.f);
			normals[3] = Vec3(0.f, 0.f, -1.f);

			normals[4] = Vec3(0.f, 0.f, 1.f);
			normals[5] = Vec3(0.f, 0.f, 1.f);
			normals[6] = Vec3(0.f, 0.f, 1.f);
			normals[7] = Vec3(0.f, 0.f, 1.f);

			normals[8] = Vec3(0.f, 1.f, 0.f);
			normals[9] = Vec3(0.f, 1.f, 0.f);
			normals[10] = Vec3(0.f, 1.f, 0.f);
			normals[11] = Vec3(0.f, 1.f, 0.f);

			normals[12] = Vec3(0.f, -1.f, 0.f);
			normals[13] = Vec3(0.f, -1.f, 0.f);
			normals[14] = Vec3(0.f, -1.f, 0.f);
			normals[15] = Vec3(0.f, -1.f, 0.f);

			normals[16] = Vec3(-1.f, 0.f, 0.f);
			normals[17] = Vec3(-1.f, 0.f, 0.f);
			normals[18] = Vec3(-1.f, 0.f, 0.f);
			normals[19] = Vec3(-1.f, 0.f, 0.f);

			normals[20] = Vec3(1.f, 0.f, 0.f);
			normals[21] = Vec3(1.f, 0.f, 0.f);
			normals[22] = Vec3(1.f, 0.f, 0.f);
			normals[23] = Vec3(1.f, 0.f, 0.f);
			D3DUtil::CreateVertexBufferResource(normals, mNormalUploadBuffer, mNormalBuffer);

			std::vector<Vec2> UVs(mVertexCnt);
			UVs[0] = Vec2(0.f, 1.f);
			UVs[1] = Vec2(0.f, 0.f);
			UVs[2] = Vec2(1.f, 0.f);
			UVs[3] = Vec2(1.f, 1.f);

			UVs[4] = Vec2(1.f, 1.f);
			UVs[5] = Vec2(0.f, 1.f);
			UVs[6] = Vec2(0.f, 0.f);
			UVs[7] = Vec2(1.f, 0.f);

			UVs[8] = Vec2(0.f, 1.f);
			UVs[9] = Vec2(0.f, 0.f);
			UVs[10] = Vec2(1.f, 0.f);
			UVs[11] = Vec2(1.f, 1.f);

			UVs[12] = Vec2(1.f, 1.f);
			UVs[13] = Vec2(0.f, 1.f);
			UVs[14] = Vec2(0.f, 0.f);
			UVs[15] = Vec2(1.f, 0.f);

			UVs[16] = Vec2(0.f, 1.f);
			UVs[17] = Vec2(0.f, 0.f);
			UVs[18] = Vec2(1.f, 0.f);
			UVs[19] = Vec2(1.f, 1.f);

			UVs[20] = Vec2(0.f, 1.f);
			UVs[21] = Vec2(0.f, 0.f);
			UVs[22] = Vec2(1.f, 0.f);
			UVs[23] = Vec2(1.f, 1.f);

			D3DUtil::CreateVertexBufferResource(UVs, mUV0UploadBuffer, mUV0Buffer);
		}
	}
	else {
		mVertexCnt = 8;
		mIndexCnt = 24;

		vertices.resize(mVertexCnt);

		vertices[0] = Vec3(-x, -y, -z);
		vertices[1] = Vec3(x, -y, -z);
		vertices[2] = Vec3(x, -y, z);
		vertices[3] = Vec3(-x, -y, z);
		vertices[4] = Vec3(-x, y, -z);
		vertices[5] = Vec3(x, y, -z);
		vertices[6] = Vec3(x, y, z);
		vertices[7] = Vec3(-x, y, z);

		indices.resize(mIndexCnt);
		indices = {
			0, 1, 1, 2, 2, 3, 3, 0, // Bottom face
			4, 5, 5, 6, 6, 7, 7, 4, // Top face
			0, 4, 1, 5, 2, 6, 3, 7, // Connecting lines
		};
	}


	D3DUtil::CreateVertexBufferResource(vertices, mVertexUploadBuffer, mVertexBuffer);

	CreateVertexBufferViews();
	CreateIndexBufferView(indices);
}

void ModelObjectMesh::CreatePlaneMesh(float width, float depth, bool isLine)
{
	const float x = width * 0.5f, z = depth * 0.5f;

	std::vector<Vec3> vertices;
	std::vector<Vec2> UV0;

	if (!isLine) {
		mVertexCnt = 6;

		vertices.resize(mVertexCnt);
		vertices[0] = Vec3(+x, -z, 0.f);
		vertices[1] = Vec3(+x, +z, 0.f);
		vertices[2] = Vec3(-x, +z, 0.f);

		vertices[3] = Vec3(+x, -z, 0.f);
		vertices[4] = Vec3(-x, +z, 0.f);
		vertices[5] = Vec3(-x, -z, 0.f);

		UV0.resize(mVertexCnt);
		UV0[0] = Vec2(0.f, 1.f);
		UV0[1] = Vec2(0.f, 0.f);
		UV0[2] = Vec2(1.f, 0.f);

		UV0[3] = Vec2(0.f, 1.f);
		UV0[4] = Vec2(1.f, 0.f);
		UV0[5] = Vec2(1.f, 1.f);

		D3DUtil::CreateVertexBufferResource(UV0, mUV0UploadBuffer, mUV0Buffer);
	}
	else {
		mVertexCnt = 8;

		vertices.resize(mVertexCnt);
		vertices[0] = Vec3(-x, 0.f, -z);
		vertices[1] = Vec3(-x, 0.f, +z);

		vertices[2] = Vec3(-x, 0.f, +z);
		vertices[3] = Vec3(+x, 0.f, +z);

		vertices[4] = Vec3(+x, 0.f, +z);
		vertices[5] = Vec3(+x, 0.f, -z);

		vertices[6] = Vec3(+x, 0.f, -z);
		vertices[7] = Vec3(-x, 0.f, -z);
	}

	D3DUtil::CreateVertexBufferResource(vertices, mVertexUploadBuffer, mVertexBuffer);

	CreateVertexBufferViews();
}


void ModelObjectMesh::CreateSphereMesh(float radius, int numSegments, bool isLine)
{
	// Calculate the number of vertices and indices needed
	const int numVertices = (numSegments + 1) * (numSegments + 1);

	int numIndices{};
	if (!isLine) {
		numIndices = numSegments * numSegments * 6;
	}
	else {
		numIndices = numSegments * numSegments * 2 * 4;
	}

	mVertexCnt = numVertices;
	mIndexCnt = numIndices;

	std::vector<Vec3> vertices;
	std::vector<UINT> indices;

	// Create vertices for the sphere
	vertices.resize(numVertices);
	float phi, theta;
	float phiStep = Math::kPI / numSegments;
	float thetaStep = 2.0f * Math::kPI / numSegments;
	int vertexIndex = 0;

	for (int i = 0; i <= numSegments; ++i) {
		phi = i * phiStep;
		for (int j = 0; j <= numSegments; ++j) {
			theta = j * thetaStep;
			float x = radius * sinf(phi) * cosf(theta);
			float y = radius * cosf(phi);
			float z = radius * sinf(phi) * sinf(theta);
			vertices[vertexIndex++] = Vec3(x, y, z);
		}
	}

	// Create indices for the sphere
	indices.resize(numIndices);
	int index = 0;

	if (!isLine) {
		for (int i = 0; i < numSegments; ++i) {
			for (int j = 0; j < numSegments; ++j) {
				int vertexIndex0 = i * (numSegments + 1) + j;
				int vertexIndex1 = vertexIndex0 + 1;
				int vertexIndex2 = (i + 1) * (numSegments + 1) + j;
				int vertexIndex3 = vertexIndex2 + 1;
				indices[index++] = vertexIndex0;
				indices[index++] = vertexIndex1;
				indices[index++] = vertexIndex2;
				indices[index++] = vertexIndex1;
				indices[index++] = vertexIndex3;
				indices[index++] = vertexIndex2;
			}
		}
	}
	else {
		// Create vertical lines
		for (int i = 0; i <= numSegments; ++i) {
			for (int j = 0; j < numSegments; ++j) {
				int vertexIndex0 = i * (numSegments + 1) + j;
				int vertexIndex1 = vertexIndex0 + 1;
				indices[index++] = vertexIndex0;
				indices[index++] = vertexIndex1;
			}
		}

		// Create horizontal lines
		for (int j = 0; j <= numSegments; ++j) {
			for (int i = 0; i < numSegments; ++i) {
				int vertexIndex0 = i * (numSegments + 1) + j;
				int vertexIndex1 = (i + 1) * (numSegments + 1) + j;
				indices[index++] = vertexIndex0;
				indices[index++] = vertexIndex1;
			}
		}
	}

	D3DUtil::CreateVertexBufferResource(vertices, mVertexUploadBuffer, mVertexBuffer);

	CreateVertexBufferViews();
	CreateIndexBufferView(indices);
}

void ModelObjectMesh::CreateRectangleMesh()
{
	mVertexCnt = 4;
	mIndexCnt = 6;

	float w2 = 0.5f;
	float h2 = 0.5f;

	std::vector<Vec3> vertices(mVertexCnt);
	vertices[0] = Vec3(-w2, -h2, 0);
	vertices[1] = Vec3(-w2, +h2, 0);
	vertices[2] = Vec3(+w2, +h2, 0);
	vertices[3] = Vec3(+w2, -h2, 0);

	std::vector<Vec2> uvs(mVertexCnt);
	uvs[0] = Vec2(0.f, 1.f);
	uvs[1] = Vec2(0.f, 0.f);
	uvs[2] = Vec2(1.f, 0.f);
	uvs[3] = Vec2(1.f, 1.f);

	std::vector<UINT> indices(mIndexCnt);
	indices[0] = 0; indices[1] = 1; indices[2] = 2;
	indices[3] = 0; indices[4] = 2; indices[5] = 3;

	D3DUtil::CreateVertexBufferResource(uvs, mUV0UploadBuffer, mUV0Buffer);
	D3DUtil::CreateVertexBufferResource(vertices, mVertexUploadBuffer, mVertexBuffer);

	CreateVertexBufferViews();
	CreateIndexBufferView(indices);
}

void ModelObjectMesh::CreatePointMesh()
{
	mVertexCnt = 1;
	mIndexCnt = 1;

	std::vector<Vec3> vertices(mVertexCnt);
	vertices[0] = Vec3(0, 0, 0);

	std::vector<Vec2> uvs(mVertexCnt);
	uvs[0] = Vec2(0.5f, 0.5f);

	std::vector<UINT> indices(mIndexCnt);
	indices[0] = 0;

	D3DUtil::CreateVertexBufferResource(uvs, mUV0UploadBuffer, mUV0Buffer);
	D3DUtil::CreateVertexBufferResource(vertices, mVertexUploadBuffer, mVertexBuffer);

	CreateVertexBufferViews();
	CreateIndexBufferView(indices);
}
#pragma endregion





#pragma region MergedMesh
MergedMesh::MergedMesh()
{
	mMeshBuffer = std::make_unique<MeshBuffer>();
}

rsptr<Texture> MergedMesh::GetTexture() const
{
	return mFrameMeshInfo.front().Materials.front()->mTextures[static_cast<UINT8>(TextureMap::DiffuseMap0)];
}

// mMeshBuffer와 mFrameMeshInfo에 [mesh] 및 [materials] 정보들을 추가(병합)한다.
void MergedMesh::MergeMesh(sptr<MeshLoadInfo>& mesh, std::vector<sptr<Material>>& materials)
{
	// 각 프레임마다 메쉬 정보(FrameMeshInfo)를 가져야 하기에 최종적으로 mFrameMeshInfo에 이를 추가한다.
	FrameMeshInfo modelMeshInfo{};

	// 메쉬가 없으면, 재질도 없다. -> 아무 정보 없는 FrameInfo를 삽입 후 종료한다.
	if (!mesh) {
		mFrameMeshInfo.emplace_back();
		return;
	}

	// material 정보를 추가한다.
	modelMeshInfo.Materials = std::move(materials);

	modelMeshInfo.SkinMesh = mesh->SkinMesh;

	// set vertexCnt
	modelMeshInfo.VertexCnt = (UINT)mesh->Buffer.Vertices.size();
	mVertexCnt             += modelMeshInfo.VertexCnt;

	// copy vertices info to [mMeshBuffer]
	CopyBack(mesh->Buffer.Vertices, mMeshBuffer->Vertices);
	CopyBack(mesh->Buffer.Normals,  mMeshBuffer->Normals);
	if (modelMeshInfo.Materials.front()->mTextures[static_cast<UINT8>(TextureMap::DiffuseMap0)]) {	// texture가 있다면, Tangents, BiTangents, UVs를 가져온다
		CopyBack(mesh->Buffer.Tangents,	  mMeshBuffer->Tangents);
		CopyBack(mesh->Buffer.BiTangents, mMeshBuffer->BiTangents);
		CopyBack(mesh->Buffer.UVs0,		  mMeshBuffer->UVs0);
	}

	// 스키닝 정보가 있다면 Bone 정보들을 추가한다.
	if (mesh->SkinMesh) {
		CopyBack(mesh->Buffer.BoneIndices, mMeshBuffer->BoneIndices);
		CopyBack(mesh->Buffer.BoneWeights, mMeshBuffer->BoneWeights);
	}

	// merge sub meshes
	MergeSubMeshes(mesh, modelMeshInfo);

	mFrameMeshInfo.emplace_back(modelMeshInfo);

	// unlink
	mesh = nullptr;
	materials.clear();
}

void MergedMesh::StopMerge()
{
	/* 기존 resource(buffer) 정보를 초기화한다. (불필요 데이터 삽입 방지) */
	mVertexBuffer    = nullptr;
	mNormalBuffer    = nullptr;
	mTangentBuffer   = nullptr;
	mBiTangentBuffer = nullptr;
	mUV0Buffer       = nullptr;
	mUV1Buffer       = nullptr;
	mIndexBuffer     = nullptr;

	/* 모든 메쉬들의 정보가 저장된 버퍼의 내용을 resource(buffer)로 생성한다. */
	D3DUtil::CreateVertexBufferResource(mMeshBuffer->Vertices, mVertexUploadBuffer, mVertexBuffer);
	D3DUtil::CreateVertexBufferResource(mMeshBuffer->UVs0, mUV0UploadBuffer, mUV0Buffer);
	D3DUtil::CreateVertexBufferResource(mMeshBuffer->UVs1, mUV1UploadBuffer, mUV1Buffer);
	D3DUtil::CreateVertexBufferResource(mMeshBuffer->Normals, mNormalUploadBuffer, mNormalBuffer);
	D3DUtil::CreateVertexBufferResource(mMeshBuffer->Tangents, mTangentUploadBuffer, mTangentBuffer);
	D3DUtil::CreateVertexBufferResource(mMeshBuffer->BiTangents, mBiTangentUploadBuffer, mBiTangentBuffer);
	D3DUtil::CreateVertexBufferResource(mMeshBuffer->BoneIndices, mBoneIndexUploadBuffer, mBoneIndexBuffer);
	D3DUtil::CreateVertexBufferResource(mMeshBuffer->BoneWeights, mBoneWeightUploadBuffer, mBoneWeightBuffer);

	// resource(buffer)에 따른 view를 생성한다.
	CreateVertexBufferViews();
	CreateIndexBufferView(mMeshBuffer->Indices);

	// 메쉬 버퍼는 더 이상 필요 없으므로 해제한다.
	mMeshBuffer = nullptr;
}

void MergedMesh::UpdateMaterialBuffer()
{
	for (const auto& meshInfo : mFrameMeshInfo) {
		for (const auto& material : meshInfo.Materials) {
			material->UpdateShaderVars();
		}
	}
}

//void UpdateShaderVars(const Matrix& transform)
//{
//	Scene::I->SetGraphicsRoot32BitConstants(RootParam::GameObjectInfo, XMMatrix::Transpose(transform), 0);
//}


void MergedMesh::Render(const GameObject* object) const
{
	// copy
	Render(object->GetMergedTransform());
}

void MergedMesh::Render(const ObjectPool* objectPool) const
{
	// copy
	if (!objectPool) {
		return;
	}

	UINT instanceCnt = objectPool->GetInstanceCnt();
	if (instanceCnt <= 0) {
		return;
	}

	objectPool->UpdateShaderVars();
	Render(objectPool->GetMergedTransform(), instanceCnt);
}


void MergedMesh::RenderSprite(const GameObject* object) const
{
	//constexpr int kRootIndex{ 0 };
	//if (!HasMesh(kRootIndex)) {	// 스프라이트는 루트의 메쉬를 사용해 렌더링한다.
	//	return;
	//}

	//CMD_LIST->IASetVertexBuffers(mSlot, (UINT)mVertexBufferViews.size(), mVertexBufferViews.data());
	//CMD_LIST->IASetIndexBuffer(&mIndexBufferView);

	//constexpr UINT kTransformIndex{ 0 };
	//const FrameMeshInfo& modelMeshInfo = mFrameMeshInfo[kTransformIndex];

	//object->GetComponent<Script_Sprite>()->UpdateSpriteVariable(modelMeshInfo.Materials.front()->mMatIndex);

	//constexpr UINT kIndexCnt{ 6 };
	//CMD_LIST->DrawIndexedInstanced(kIndexCnt, 1, 0, 0, 0);
}

// sub meshes는 한 정점 버퍼에 대한 여러개의 indices만을 가진다.
// -> 각 indices의 내용과 그 개수를 저장해 사용하도록 한다.
void MergedMesh::MergeSubMeshes(rsptr<MeshLoadInfo> mesh, FrameMeshInfo& modelMeshInfo)
{
	const int subMeshCnt = mesh->SubMeshCnt;
	modelMeshInfo.IndicesCnts.resize(subMeshCnt);

	for (int i = 0; i < subMeshCnt; ++i) {
		std::vector<UINT>& indices = mesh->SubSetIndices[i];
		modelMeshInfo.IndicesCnts[i] = (UINT)indices.size();
		CopyBack(indices, mMeshBuffer->Indices);
	}

	for (UINT indexCnt : modelMeshInfo.IndicesCnts) {
		mIndexCnt += indexCnt;
	}
}

void MergedMesh::Render(const std::vector<const Transform*>& mergedTransform, UINT instanceCnt) const
{
	CMD_LIST->IASetVertexBuffers(mSlot, (UINT)mVertexBufferViews.size(), mVertexBufferViews.data());
	CMD_LIST->IASetIndexBuffer(&mIndexBufferView);

	UINT indexLocation{ 0 };
	UINT vertexLocation{ 0 };
	const UINT transformCnt = (UINT)mergedTransform.size();

	// 최상위 부모의 mUseObjCB가 true여야 객체 파괴시 오브젝트 인덱스 반환 가능
	const Transform* root = mergedTransform[0];
	root->SetUseObjCB(true);

	const float deathElapsed = root->mObjectCB.DeathElapsed;

	for (UINT transformIndex = 0; transformIndex < transformCnt; ++transformIndex) {
		const Transform* transform = mergedTransform[transformIndex];

		if (!HasMesh(transformIndex)) {
			continue;
		}

		const FrameMeshInfo& modelMeshInfo = mFrameMeshInfo[transformIndex];
		if (modelMeshInfo.SkinMesh) {
			modelMeshInfo.SkinMesh->UpdateShaderVariables();
		}

		UINT vertexCnt = modelMeshInfo.VertexCnt;
		UINT mat{ 0 };

		ObjectConstants objectCB;
		objectCB.DeathElapsed = deathElapsed;

		for (UINT indexCnt : modelMeshInfo.IndicesCnts) {
			objectCB.MtxWorld = transform->GetWorldTransform().Transpose();
			objectCB.MatIndex = modelMeshInfo.Materials[mat]->mMatIndex;

			// 서브 메쉬에 대하여 각각의 머티리얼 인덱스를 오브젝트 상수 버퍼에 설정해야 한다.
			// 때문에 transform의 데이터를 여러 번 설정하되 머티리얼은 설정하지 않는다. 
			transform->UpdateShaderVars(objectCB, mat);

			CMD_LIST->DrawIndexedInstanced(indexCnt, instanceCnt, indexLocation, vertexLocation, 0);
			indexLocation += indexCnt;
			mat++;
		}

		vertexLocation += vertexCnt;
	}
}
#pragma endregion


#pragma region Avatar
BoneType Avatar::GetBoneType(const std::string& boneName) const
{
	if (mBoneTypes.contains(boneName)) {
		return mBoneTypes.at(boneName);
	}
	
	return BoneType::None;
}

void Avatar::SetBoneType(const std::string& boneName, const std::string& boneType)
{
	BoneType type = BoneType::None;

	switch (Hash(boneType)) {
		// Body
		case Hash("Hips"):
			type = BoneType::Hips;
			break;
		case Hash("Spine"):
			type = BoneType::Spine;
			break;
		case Hash("Chest"):
			type = BoneType::Chest;
			break;
		case Hash("UpperChest"):
			type = BoneType::UpperChest;
			break;

			// Left Arm
		case Hash("LeftShoulder"):
			type = BoneType::LeftShoulder;
			break;
		case Hash("LeftUpperArm"):
			type = BoneType::LeftUpperArm;
			break;
		case Hash("LeftLowerArm"):
			type = BoneType::LeftLowerArm;
			break;
		case Hash("LeftHand"):
			type = BoneType::LeftHand;
			break;

			// Right Arm
		case Hash("RightShoulder"):
			type = BoneType::RightShoulder;
			break;
		case Hash("RightUpperArm"):
			type = BoneType::RightUpperArm;
			break;
		case Hash("RightLowerArm"):
			type = BoneType::RightLowerArm;
			break;
		case Hash("RightHand"):
			type = BoneType::RightHand;
			break;

			// Left Leg
		case Hash("LeftUpperLeg"):
			type = BoneType::LeftUpperLeg;
			break;
		case Hash("LeftLowerLeg"):
			type = BoneType::LeftLowerLeg;
			break;
		case Hash("LeftFoot"):
			type = BoneType::LeftFoot;
			break;
		case Hash("LeftToes"):
			type = BoneType::LeftToes;
			break;

			// Right Leg
		case Hash("RightUpperLeg"):
			type = BoneType::RightUpperLeg;
			break;
		case Hash("RightLowerLeg"):
			type = BoneType::RightLowerLeg;
			break;
		case Hash("RightFoot"):
			type = BoneType::RightFoot;
			break;
		case Hash("RightToes"):
			type = BoneType::RightToes;
			break;

			// Head
		case Hash("Neck"):
			type = BoneType::Neck;
			break;
		case Hash("Head"):
			type = BoneType::Head;
			break;
		case Hash("LeftEye"):
			type = BoneType::LeftEye;
			break;
		case Hash("RightEye"):
			type = BoneType::RightEye;
			break;
		case Hash("Jaw"):
			type = BoneType::Jaw;
			break;

			// Left Hand
		case Hash("Left Thumb Proximal"):
			type = BoneType::LeftThumbProximal;
			break;
		case Hash("Left Thumb Intermediate"):
			type = BoneType::LeftThumbIntermediate;
			break;
		case Hash("Left Thumb Distal"):
			type = BoneType::LeftThumbDistal;
			break;

		case Hash("Left Index Proximal"):
			type = BoneType::LeftIndexProximal;
			break;
		case Hash("Left Index Intermediate"):
			type = BoneType::LeftIndexIntermediate;
			break;
		case Hash("Left Index Distal"):
			type = BoneType::LeftIndexDistal;
			break;

		case Hash("Left Middle Proximal"):
			type = BoneType::LeftMiddleProximal;
			break;
		case Hash("Left Middle Intermediate"):
			type = BoneType::LeftMiddleIntermediate;
			break;
		case Hash("Left Middle Distal"):
			type = BoneType::LeftMiddleDistal;
			break;

		case Hash("Left Ring Proximal"):
			type = BoneType::LeftRingProximal;
			break;
		case Hash("Left Ring Intermediate"):
			type = BoneType::LeftRingIntermediate;
			break;
		case Hash("Left Ring Distal"):
			type = BoneType::LeftRingDistal;
			break;

		case Hash("Left Little Proximal"):
			type = BoneType::LeftLittleProximal;
			break;
		case Hash("Left Little Intermediate"):
			type = BoneType::LeftLittleIntermediate;
			break;
		case Hash("Left Little Distal"):
			type = BoneType::LeftLittleDistal;
			break;

			// Right Hand
		case Hash("Right Thumb Proximal"):
			type = BoneType::RightThumbProximal;
			break;
		case Hash("Right Thumb Intermediate"):
			type = BoneType::RightThumbIntermediate;
			break;
		case Hash("Right Thumb Distal"):
			type = BoneType::RightThumbDistal;
			break;

		case Hash("Right Index Proximal"):
			type = BoneType::RightIndexProximal;
			break;
		case Hash("Right Index Intermediate"):
			type = BoneType::RightIndexIntermediate;
			break;
		case Hash("Right Index Distal"):
			type = BoneType::RightIndexDistal;
			break;

		case Hash("Right Middle Proximal"):
			type = BoneType::RightMiddleProximal;
			break;
		case Hash("Right Middle Intermediate"):
			type = BoneType::RightMiddleIntermediate;
			break;
		case Hash("Right Middle Distal"):
			type = BoneType::RightMiddleDistal;
			break;

		case Hash("Right Ring Proximal"):
			type = BoneType::RightRingProximal;
			break;
		case Hash("Right Ring Intermediate"):
			type = BoneType::RightRingIntermediate;
			break;
		case Hash("Right Ring Distal"):
			type = BoneType::RightRingDistal;
			break;

		case Hash("Right Little Proximal"):
			type = BoneType::RightLittleProximal;
			break;
		case Hash("Right Little Intermediate"):
			type = BoneType::RightLittleIntermediate;
			break;
		case Hash("Right Little Distal"):
			type = BoneType::RightLittleDistal;
			break;
		default:
			break;
	}

	mBoneTypes[boneName] = type;
}

#pragma endregion



#pragma region SkinMesh
BoneType SkinMesh::GetBoneType(int boneIndex) const
{
	if (boneIndex < 0 || boneIndex >= mBoneTypes.size()) {
		return BoneType::None;
	}

	return mBoneTypes.at(boneIndex);
}

HumanBone SkinMesh::GetHumanBone(int boneIndex) const
{
	switch (mBoneTypes[boneIndex]) {
	case BoneType::Hips:
	case BoneType::Spine:
		return HumanBone::Root;

	case BoneType::Chest:
	case BoneType::UpperChest:
		return HumanBone::Body;

	case BoneType::LeftShoulder:
	case BoneType::LeftUpperArm:
	case BoneType::LeftLowerArm:
	case BoneType::LeftHand:
		return HumanBone::LeftArm;

	case BoneType::RightShoulder:
	case BoneType::RightUpperArm:
	case BoneType::RightLowerArm:
	case BoneType::RightHand:
		return HumanBone::RightArm;

	case BoneType::LeftUpperLeg :
	case BoneType::LeftLowerLeg:
	case BoneType::LeftFoot:
	case BoneType::LeftToes:
		return HumanBone::LeftLeg;

	case BoneType::RightUpperLeg:
	case BoneType::RightLowerLeg:
	case BoneType::RightFoot:
	case BoneType::RightToes:
		return HumanBone::RightLeg;

	case BoneType::Neck:
	case BoneType::Head:
	case BoneType::LeftEye:
	case BoneType::RightEye:
	case BoneType::Jaw:
		return HumanBone::Head;

	case BoneType::LeftThumbProximal:
	case BoneType::LeftThumbIntermediate:
	case BoneType::LeftThumbDistal:
	case BoneType::LeftIndexProximal:
	case BoneType::LeftIndexIntermediate:
	case BoneType::LeftIndexDistal:
	case BoneType::LeftMiddleProximal:
	case BoneType::LeftMiddleIntermediate:
	case BoneType::LeftMiddleDistal:
	case BoneType::LeftRingProximal:
	case BoneType::LeftRingIntermediate:
	case BoneType::LeftRingDistal:
	case BoneType::LeftLittleProximal:
	case BoneType::LeftLittleIntermediate:
	case BoneType::LeftLittleDistal:
		return HumanBone::LeftHand;

	case BoneType::RightThumbProximal:
	case BoneType::RightThumbIntermediate:
	case BoneType::RightThumbDistal:
	case BoneType::RightIndexProximal:
	case BoneType::RightIndexIntermediate:
	case BoneType::RightIndexDistal:
	case BoneType::RightMiddleProximal:
	case BoneType::RightMiddleIntermediate:
	case BoneType::RightMiddleDistal:
	case BoneType::RightRingProximal:
	case BoneType::RightRingIntermediate:
	case BoneType::RightRingDistal:
	case BoneType::RightLittleProximal:
	case BoneType::RightLittleIntermediate:
	case BoneType::RightLittleDistal:
		return HumanBone::LeftHand;

	default:
		return HumanBone::None;
	}
}

void SkinMesh::UpdateShaderVariables()
{
	SkinnedConstants skinnedConstatnts{};
	
	for (int i = 0; i < (*mBoneFrames).size(); ++i)
	{
		Matrix transform = mBoneOffsets[i] * (*mBoneFrames)[i]->GetWorldTransform();
		XMStoreFloat4x4(&skinnedConstatnts.BoneTransforms[i], XMMatrixTranspose(XMLoadFloat4x4(&transform)));
	}

	// TODO : Memory Leak
	int index = (*mBoneFrames)[0]->GetObjCBIndex();
	FRAME_RESOURCE_MGR->CopyData(index, skinnedConstatnts);
	(*mBoneFrames)[0]->SetObjCBIndex(index);

	DXGIMgr::I->SetGraphicsRootConstantBufferView(RootParam::SkinMesh, FRAME_RESOURCE_MGR->GetSKinMeshCBGpuAddr(index));

}
#pragma endregion