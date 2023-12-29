#include "stdafx.h"
#include "Mesh.h"
#include "DXGIMgr.h"

#include "Object.h"
#include "Model.h"
#include "Shader.h"
#include "Scene.h"


#include "Script_Sprite.h"




namespace MeshRenderer {
	namespace {
		uptr<ModelObjectMesh> boxMesh{};
		uptr<ModelObjectMesh> sphereMesh{};
	}

	void BuildMeshes()
	{
		boxMesh = std::make_unique<ModelObjectMesh>();
		boxMesh->CreateCubeMesh(1.0f, 1.0f, 1.0f, false, true);

		sphereMesh = std::make_unique<ModelObjectMesh>();
		sphereMesh->CreateSphereMesh(1.0f, true);
	}

	void ReleaseStaticUploadBuffers()
	{
		boxMesh->ReleaseUploadBuffers();
		sphereMesh->ReleaseUploadBuffers();
	}

	void Render(const MyBoundingOrientedBox box)
	{
		const Vec3 center = box.Center;
		const Vec3 extents = box.Extents;
		const XMFLOAT4 orientation = box.Orientation;
		const XMVECTOR quaternion = XMLoadFloat4(&orientation);

		const XMMATRIX scaleMatrix = XMMatrixScaling(extents.x * 2, extents.y * 2, extents.z * 2);
		const XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(quaternion);
		const XMMATRIX translateMatrix = XMMatrixTranslation(center.x, center.y, center.z);

		XMMATRIX matrix = scaleMatrix;
		matrix = XMMatrixMultiply(matrix, rotationMatrix);
		matrix = XMMatrixMultiply(matrix, translateMatrix);

		Transform::UpdateShaderVariables(matrix);
		boxMesh->Render();
	}

	void Render(const MyBoundingSphere bs)
	{
		const Vec3 center = bs.Center;
		const float radius = bs.Radius;

		const XMMATRIX scaleMatrix = XMMatrixScaling(radius / 2, radius / 2, radius / 2);
		const XMMATRIX translateMatrix = XMMatrixTranslation(center.x, center.y, center.z);

		XMMATRIX matrix = scaleMatrix;
		matrix = XMMatrixMultiply(matrix, translateMatrix);

		Transform::UpdateShaderVariables(matrix);
		sphereMesh->Render();
	}

	void Release()
	{
		boxMesh = nullptr;
		sphereMesh = nullptr;
	}

}










// [ Mesh ] //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Mesh::CreateVertexBufferViews()
{
	BufferViews bufferViews{};
	bufferViews.vertexBuffer = mVertexBuffer;
	bufferViews.normalBuffer = mNormalBuffer;
	bufferViews.UV0Buffer = mUV0Buffer;
	bufferViews.UV1Buffer = mUV1Buffer;
	bufferViews.tangentBuffer = mTangentBuffer;
	bufferViews.biTangentBuffer = mBiTangentBuffer;

	D3DUtil::CreateVertexBufferViews(mVertexBufferViews, mVertexCount, bufferViews);
}

void Mesh::ReleaseUploadBuffers()
{
	mVertexUploadBuffer = nullptr;
	mNormalUploadBuffer = nullptr;

	mUV0UploadBuffer = nullptr;
	mUV1UploadBuffer = nullptr;
	mTangentUploadBuffer = nullptr;
	mBiTangentUploadBuffer = nullptr;

	mIndexUploadBuffer = nullptr;
};

void Mesh::Render() const
{
	cmdList->IASetVertexBuffers(mSlot, mVertexBufferViews.size(), mVertexBufferViews.data());
	if (mIndexBuffer) {
		cmdList->IASetIndexBuffer(&mIndexBufferView);
		cmdList->DrawIndexedInstanced(mIndexCount, 1, 0, 0, 0);
	}
	else {
		cmdList->DrawInstanced(mVertexCount, 1, mOffset, 0);
	}
}

void Mesh::RenderInstanced(UINT instanceCount) const
{
	if (instanceCount <= 0) {
		return;
	}

	cmdList->IASetVertexBuffers(mSlot, mVertexBufferViews.size(), mVertexBufferViews.data());
	if (mIndexBuffer) {
		cmdList->IASetIndexBuffer(&mIndexBufferView);
		cmdList->DrawIndexedInstanced(mIndexCount, instanceCount, 0, 0, 0);
	}
	else {
		cmdList->DrawInstanced(mVertexCount, instanceCount, mOffset, 0);
	}
}




// [ ModelObjectMesh ] //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ModelObjectMesh::CreateMeshFromOBB(const BoundingOrientedBox& box)
{
	std::vector<Vec3> vertices;
	std::vector<UINT> indices;

	mVertexCount = 8;
	mIndexCount = 36;

	vertices.resize(mVertexCount);

	vertices[0] = Vector3::Add(box.Center, Vector3::Multiply(box.Extents, Vector3::LDB()));
	vertices[1] = Vector3::Add(box.Center, Vector3::Multiply(box.Extents, Vector3::RDB()));
	vertices[2] = Vector3::Add(box.Center, Vector3::Multiply(box.Extents, Vector3::LUB()));
	vertices[3] = Vector3::Add(box.Center, Vector3::Multiply(box.Extents, Vector3::RDB()));
	vertices[4] = Vector3::Add(box.Center, Vector3::Multiply(box.Extents, Vector3::LDF()));
	vertices[5] = Vector3::Add(box.Center, Vector3::Multiply(box.Extents, Vector3::RDB()));
	vertices[6] = Vector3::Add(box.Center, Vector3::Multiply(box.Extents, Vector3::LUF()));
	vertices[7] = Vector3::Add(box.Center, Vector3::Multiply(box.Extents, Vector3::RUF()));

	indices.resize(mIndexCount);
	indices = {
		0, 1, 2, 2, 1, 3, // Front face
		4, 6, 5, 5, 6, 7, // Back face
		0, 2, 4, 4, 2, 6, // Left face
		1, 5, 3, 3, 5, 7, // Right face
		0, 4, 1, 1, 4, 5, // Bottom face
		2, 3, 6, 6, 3, 7  // Top face
	};
	

	// create buffer resource //
	D3DUtil::CreateVertexBufferResource<Vec3>(vertices, mVertexUploadBuffer, mVertexBuffer);

	D3DUtil::CreateIndexBufferResource(indices, mIndexUploadBuffer, mIndexBuffer);

	// create buffer view //
	CreateVertexBufferViews();

	D3DUtil::CreateIndexBufferView(mIndexBufferView, mIndexCount, mIndexBuffer);
}

void ModelObjectMesh::CreateCubeMesh(float width, float height, float depth, bool hasTexture, bool isLine)
{
	const float x = width * 0.5f, y = height * 0.5f, z = depth * 0.5f;

	std::vector<Vec3> vertices;
	std::vector<UINT> indices;

	if (!isLine) {
		mIndexCount = 36;
		indices.resize(mIndexCount);

		if (!hasTexture) {
			mVertexCount = 8;
			vertices.resize(mVertexCount);
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
			mVertexCount = 24;
			vertices.resize(mVertexCount);
			vertices[0] = Vec3(+x, -y, +z);
			vertices[1] = Vec3(+x, +y, +z);
			vertices[2] = Vec3(-x, +y, +z);
			vertices[3] = Vec3(-x, -y, +z);
			vertices[4] = Vec3(-x, +y, -z);
			vertices[5] = Vec3(+x, +y, -z);
			vertices[6] = Vec3(+x, -y, -z);
			vertices[7] = Vec3(-x, -y, -z);
			vertices[8] = Vec3(-x, +y, +z);
			vertices[9] = Vec3(+x, +y, +z);
			vertices[10] = Vec3(+x, +y, -z);
			vertices[11] = Vec3(-x, +y, -z);
			vertices[12] = Vec3(+x, -y, -z);
			vertices[13] = Vec3(+x, -y, +z);
			vertices[14] = Vec3(-x, -y, +z);
			vertices[15] = Vec3(-x, -y, -z);
			vertices[16] = Vec3(-x, +y, +z);
			vertices[17] = Vec3(-x, +y, -z);
			vertices[18] = Vec3(-x, -y, -z);
			vertices[19] = Vec3(-x, -y, +z);
			vertices[20] = Vec3(+x, +y, -z);
			vertices[21] = Vec3(+x, +y, +z);
			vertices[22] = Vec3(+x, -y, +z);
			vertices[23] = Vec3(+x, -y, -z);

			indices = {
				0, 1, 2, 0, 2, 3,
				4, 5, 6, 4, 6, 7,
				8, 9, 10, 8, 10, 11,
				12, 13, 14, 12, 14, 15,
				16, 17, 18, 16, 18, 19,
				20, 21, 22, 20, 22, 23
			};

			std::vector<Vec2> UVs(mVertexCount);
			UVs[0] = Vec2(0.0f, 1.0f);
			UVs[1] = Vec2(0.0f, 0.0f);
			UVs[2] = Vec2(1.0f, 0.0f);
			UVs[3] = Vec2(1.0f, 1.0f);
			UVs[4] = Vec2(0.0f, 0.0f);
			UVs[5] = Vec2(1.0f, 0.0f);
			UVs[6] = Vec2(1.0f, 1.0f);
			UVs[7] = Vec2(0.0f, 1.0f);
			UVs[8] = Vec2(0.0f, 0.0f);
			UVs[9] = Vec2(1.0f, 0.0f);
			UVs[10] = Vec2(0.0f, 1.0f);
			UVs[11] = Vec2(1.0f, 1.0f);
			UVs[12] = Vec2(0.0f, 0.0f);
			UVs[13] = Vec2(0.0f, 1.0f);
			UVs[14] = Vec2(1.0f, 1.0f);
			UVs[15] = Vec2(1.0f, 0.0f);
			UVs[16] = Vec2(0.0f, 0.0f);
			UVs[17] = Vec2(1.0f, 0.0f);
			UVs[18] = Vec2(1.0f, 1.0f);
			UVs[19] = Vec2(0.0f, 1.0f);
			UVs[20] = Vec2(0.0f, 0.0f);
			UVs[21] = Vec2(1.0f, 0.0f);
			UVs[22] = Vec2(1.0f, 1.0f);
			UVs[23] = Vec2(0.0f, 1.0f);

			D3DUtil::CreateVertexBufferResource<Vec2>(UVs, mUV0UploadBuffer, mUV0Buffer);
		}
	}
	else {
		mVertexCount = 8;
		mIndexCount = 24;

		vertices.resize(mVertexCount);

		vertices[0] = Vec3(-x, -y, -z);
		vertices[1] = Vec3(x, -y, -z);
		vertices[2] = Vec3(x, -y, z);
		vertices[3] = Vec3(-x, -y, z);
		vertices[4] = Vec3(-x, y, -z);
		vertices[5] = Vec3(x, y, -z);
		vertices[6] = Vec3(x, y, z);
		vertices[7] = Vec3(-x, y, z);

		indices.resize(mIndexCount);
		indices = {
			0, 1, 1, 2, 2, 3, 3, 0, // Bottom face
			4, 5, 5, 6, 6, 7, 7, 4, // Top face
			0, 4, 1, 5, 2, 6, 3, 7, // Connecting lines
		};
	}
	

	// create buffer resource //
	D3DUtil::CreateVertexBufferResource<Vec3>(vertices, mVertexUploadBuffer, mVertexBuffer);
	D3DUtil::CreateIndexBufferResource(indices, mIndexUploadBuffer, mIndexBuffer);

	// create buffer view //
	CreateVertexBufferViews();

	D3DUtil::CreateIndexBufferView(mIndexBufferView, mIndexCount, mIndexBuffer);
}

void ModelObjectMesh::CreatePlaneMesh(float width, float depth, bool isLine)
{
	const float x = width * 0.5f, z = depth * 0.5f;

	std::vector<Vec3> vertices;
	std::vector<Vec2> UV0;

	if (!isLine) {
		mVertexCount = 6;

		vertices.resize(mVertexCount);
		vertices[0] = Vec3(+x, -z, 0.f);
		vertices[1] = Vec3(+x, +z, 0.f);
		vertices[2] = Vec3(-x, +z, 0.f);

		vertices[3] = Vec3(+x, -z, 0.f);
		vertices[4] = Vec3(-x, +z, 0.f);
		vertices[5] = Vec3(-x, -z, 0.f);

		UV0.resize(mVertexCount);
		UV0[0] = Vec2(0.0f, 1.0f);
		UV0[1] = Vec2(0.0f, 0.0f);
		UV0[2] = Vec2(1.0f, 0.0f);

		UV0[3] = Vec2(0.0f, 1.0f);
		UV0[4] = Vec2(1.0f, 0.0f);
		UV0[5] = Vec2(1.0f, 1.0f);

		D3DUtil::CreateVertexBufferResource<Vec2>(UV0, mUV0UploadBuffer, mUV0Buffer);
	}
	else {
		mVertexCount = 8;

		vertices.resize(mVertexCount);
		vertices[0] = Vec3(-x, 0.0f, -z);
		vertices[1] = Vec3(-x, 0.0f, +z);

		vertices[2] = Vec3(-x, 0.0f, +z);
		vertices[3] = Vec3(+x, 0.0f, +z);

		vertices[4] = Vec3(+x, 0.0f, +z);
		vertices[5] = Vec3(+x, 0.0f, -z);

		vertices[6] = Vec3(+x, 0.0f, -z);
		vertices[7] = Vec3(-x, 0.0f, -z);
	}

	// create buffer resource //
	D3DUtil::CreateVertexBufferResource<Vec3>(vertices, mVertexUploadBuffer, mVertexBuffer);

	// create buffer view //
	CreateVertexBufferViews();
}


void ModelObjectMesh::CreateSphereMesh(float radius, bool isLine, int numSegments)
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

	mVertexCount = numVertices;
	mIndexCount = numIndices;

	std::vector<Vec3> vertices;
	std::vector<UINT> indices;

	// Create vertices for the sphere
	vertices.resize(numVertices);
	float phi, theta;
	float phiStep = PI / numSegments;
	float thetaStep = 2.0f * PI / numSegments;
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

	// Create buffer resource
	D3DUtil::CreateVertexBufferResource<Vec3>(vertices, mVertexUploadBuffer, mVertexBuffer);
	D3DUtil::CreateIndexBufferResource(indices, mIndexUploadBuffer, mIndexBuffer);

	// Create buffer view
	CreateVertexBufferViews();

	D3DUtil::CreateIndexBufferView(mIndexBufferView, mIndexCount, mIndexBuffer);
}




// [ SkyBoxMesh ] //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SkyBoxMesh::SkyBoxMesh(float width, float height, float depth)
{
	CreateMesh(width, height, depth);
}


void SkyBoxMesh::CreateMesh(float width, float height, float depth)
{
	const float x = width * 0.5f, y = height * 0.5f, z = depth * 0.5f;

	mVertexCount = 36;

	std::vector<Vec3> vertices;

	vertices.resize(mVertexCount);
	vertices[0] = Vec3(-x, +x, +x);
	vertices[1] = Vec3(+x, +x, +x);
	vertices[2] = Vec3(-x, -x, +x);
	vertices[3] = Vec3(-x, -x, +x);
	vertices[4] = Vec3(+x, +x, +x);
	vertices[5] = Vec3(+x, -x, +x);
	// Back Quad										
	vertices[6] = Vec3(+x, +x, -x);
	vertices[7] = Vec3(-x, +x, -x);
	vertices[8] = Vec3(+x, -x, -x);
	vertices[9] = Vec3(+x, -x, -x);
	vertices[10] = Vec3(-x, +x, -x);
	vertices[11] = Vec3(-x, -x, -x);
	// Left Quad										
	vertices[12] = Vec3(-x, +x, -x);
	vertices[13] = Vec3(-x, +x, +x);
	vertices[14] = Vec3(-x, -x, -x);
	vertices[15] = Vec3(-x, -x, -x);
	vertices[16] = Vec3(-x, +x, +x);
	vertices[17] = Vec3(-x, -x, +x);
	// Right Quad										
	vertices[18] = Vec3(+x, +x, +x);
	vertices[19] = Vec3(+x, +x, -x);
	vertices[20] = Vec3(+x, -x, +x);
	vertices[21] = Vec3(+x, -x, +x);
	vertices[22] = Vec3(+x, +x, -x);
	vertices[23] = Vec3(+x, -x, -x);
	// Top Quad											
	vertices[24] = Vec3(-x, +x, -x);
	vertices[25] = Vec3(+x, +x, -x);
	vertices[26] = Vec3(-x, +x, +x);
	vertices[27] = Vec3(-x, +x, +x);
	vertices[28] = Vec3(+x, +x, -x);
	vertices[29] = Vec3(+x, +x, +x);
	// Bottom Quad										
	vertices[30] = Vec3(-x, -x, +x);
	vertices[31] = Vec3(+x, -x, +x);
	vertices[32] = Vec3(-x, -x, -x);
	vertices[33] = Vec3(-x, -x, -x);
	vertices[34] = Vec3(+x, -x, +x);
	vertices[35] = Vec3(+x, -x, -x);

	// create buffer resource //
	D3DUtil::CreateVertexBufferResource<Vec3>(vertices, mVertexUploadBuffer, mVertexBuffer);

	// create buffer view //
	CreateVertexBufferViews();
}









// [ MeshLoadInfo ] //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MeshLoadInfo::~MeshLoadInfo()
{

}





// [ MergedMesh ] //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MergedMesh::MergedMesh()
{
	mMeshBuffer = std::make_unique<MeshBuffer>();
}

rsptr<Texture> MergedMesh::GetTexture() const
{
	return mModelInfo.front().mMaterials.front()->mTexture;
}

void MergedMesh::MergeSubMeshes(rsptr<MeshLoadInfo> mesh, ModelInfo& modelInfo) // merge submesh to one indices vector
{
	const UINT subMeshCount = mesh->mSubMeshCount;
	modelInfo.mIndexCounts.resize(subMeshCount);

	for (int i = 0; i < subMeshCount; ++i) {
		std::vector<UINT>& indices = mesh->mSubSetIndices[i];
		modelInfo.mIndexCounts[i] = indices.size();
		CopyBack(indices, mMeshBuffer->mIndices);
	}

	for (UINT indexCount : modelInfo.mIndexCounts) {
		mIndexCount += indexCount;
	}
}

bool MergedMesh::MergeMesh(rsptr<MeshLoadInfo> mesh, const std::vector<sptr<Material>>& materials)
{
	ModelInfo modelInfo{};

	if (!mesh) {
		mModelInfo.emplace_back(modelInfo);
		return false;
	}
	CopyBack(materials, modelInfo.mMaterials);

	// copy vertices
	const std::vector<Vec3>& meshVertices = mesh->mVertices;
	const std::vector<Vec3>& meshNormals = mesh->mNormals;

	CopyBack(meshVertices, mMeshBuffer->mVertices);
	CopyBack(meshNormals, mMeshBuffer->mNormals);

	if (modelInfo.mMaterials[0]->mTexture) {
		const std::vector<Vec3>& meshTangents = mesh->mTangents;
		const std::vector<Vec3>& meshBiTangents = mesh->mBiTangents;
		const std::vector<Vec2>& meshUVs0 = mesh->mUV0;
		//const std::vector<Vec2>& meshUVs1 = mesh->mUV1;

		CopyBack(meshTangents, mMeshBuffer->mTangents);
		CopyBack(meshBiTangents, mMeshBuffer->mBiTangents);
		CopyBack(meshUVs0, mMeshBuffer->mUVs0);
		//CopyBack(meshUVs1, mUVs1);
	}

	// set vertexCount
	modelInfo.mVertexCount = mesh->mVertices.size();
	mVertexCount += meshVertices.size();

	// merge & copy SubMeshes
	MergeSubMeshes(mesh, modelInfo);

	mModelInfo.emplace_back(modelInfo);

	return true;
}

void MergedMesh::Close()
{
	mVertexBuffer = nullptr;
	mNormalBuffer = nullptr;

	mTangentBuffer = nullptr;
	mBiTangentBuffer = nullptr;
	mUV0Buffer = nullptr;
	mUV1Buffer = nullptr;

	mIndexBuffer = nullptr;

	// create buffer resource //
	D3DUtil::CreateVertexBufferResource<Vec3>(mMeshBuffer->mVertices, mVertexUploadBuffer, mVertexBuffer);
	D3DUtil::CreateVertexBufferResource<Vec2>(mMeshBuffer->mUVs0, mUV0UploadBuffer, mUV0Buffer);
	//D3DUtil::CreateVertexBufferResource<Vec2>(mMeshBuffer->mUVs1, mUV1UploadBuffer, mUV1Buffer);
	D3DUtil::CreateVertexBufferResource<Vec3>(mMeshBuffer->mNormals, mNormalUploadBuffer, mNormalBuffer);
	D3DUtil::CreateVertexBufferResource<Vec3>(mMeshBuffer->mTangents, mTangentUploadBuffer, mTangentBuffer);
	D3DUtil::CreateVertexBufferResource<Vec3>(mMeshBuffer->mBiTangents, mBiTangentUploadBuffer, mBiTangentBuffer);
	D3DUtil::CreateIndexBufferResource(mMeshBuffer->mIndices, mIndexUploadBuffer, mIndexBuffer);

	CreateVertexBufferViews();
	D3DUtil::CreateIndexBufferView(mIndexBufferView, mIndexCount, mIndexBuffer);

	mMeshBuffer = nullptr;
}

void UpdateShaderVariable(const Vec4x4& transform)
{
	scene->SetGraphicsRoot32BitConstants(RootParam::GameObjectInfo, XMMatrix::Transpose(transform), 0);
}

void MergedMesh::Render(const std::vector<const Transform*>& mergedTransform, UINT instanceCount) const
{
	cmdList->IASetVertexBuffers(mSlot, mVertexBufferViews.size(), mVertexBufferViews.data());
	cmdList->IASetIndexBuffer(&mIndexBufferView);

	UINT indexLocation{};
	UINT vertexLocation{};
	const UINT transformCount = mergedTransform.size();

	for (int transformIndex = 0; transformIndex < transformCount; ++transformIndex) {
		if (HasMesh(transformIndex)) {
			const ModelInfo& modelInfo = mModelInfo[transformIndex];
			const Transform* transform = mergedTransform[transformIndex];

			transform->UpdateShaderVariable();

			UINT vertexCount = modelInfo.mVertexCount;
			UINT mat{};
			for (UINT indexCount : modelInfo.mIndexCounts) {
				modelInfo.mMaterials[mat++]->UpdateShaderVariable();

				cmdList->DrawIndexedInstanced(indexCount, instanceCount, indexLocation, vertexLocation, 0);
				indexLocation += indexCount;
			}

			vertexLocation += vertexCount;
		}
	}
}

void MergedMesh::Render(const GameObject* gameObject) const
{
	Render(gameObject->GetMergedTransform());
}

void MergedMesh::Render(const ObjectInstanceBuffer* instBuffer) const
{
	if (!instBuffer) {
		return;
	}

	UINT instanceCount = instBuffer->GetInstanceCount();
	if (instanceCount <= 0) {
		return;
	}

	instBuffer->UpdateShaderVariables();

	Render(instBuffer->GetMergedTransform(), instanceCount);
}


void MergedMesh::RenderSprite(const GameObject* gameObject) const
{
	constexpr int rootIndex{ 0 };
	if (!HasMesh(rootIndex)) {
		return;
	}

	cmdList->IASetVertexBuffers(mSlot, mVertexBufferViews.size(), mVertexBufferViews.data());
	cmdList->IASetIndexBuffer(&mIndexBufferView);

	constexpr UINT transformIndex{ 0 };
	const ModelInfo& modelInfo = mModelInfo[transformIndex];

	modelInfo.mMaterials.front()->UpdateShaderVariable();
	gameObject->GetComponent<Script_Sprite>()->UpdateSpriteVariable();

	constexpr UINT indexCount{ 6 };
	cmdList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}