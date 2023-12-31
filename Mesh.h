#pragma once

#pragma region Include
#include "Collider.h"
#pragma endregion


#pragma region ClassForwardDecl
class Material;
class Texture;
class Transform;
class ModelObject;
class GameObject;
class ObjectInstanceBuffer;
#pragma endregion


#pragma region EnumClass
enum class VertexType : DWORD {
	Position  = 0x01,
	Color     = 0x02,
	Normal    = 0x04,
	Tangent   = 0x08,
	BiTangent = 0x10,
	UV0       = 0x20,
	UV1       = 0x40,
};
#pragma endregion


#pragma region Struct
struct ModelInfo {
	std::vector<sptr<Material>> Materials{};
	std::vector<UINT>			IndexCounts{};
	UINT						VertexCount{};
};

struct MeshBuffer {
	std::vector<Vec3> Vertices{};
	std::vector<Vec3> Normals{};
	std::vector<Vec3> Tangents{};
	std::vector<Vec3> BiTangents{};
	std::vector<Vec2> UVs0{};
	std::vector<Vec2> UVs1{};

	std::vector<UINT> Indices{};
};

struct MeshLoadInfo {
	DWORD		Type{};
	std::string MeshName{};

	MyBoundingSphere					BS{};
	std::vector<MyBoundingOrientedBox>	OBBList{};

	int			VertexCount{};
	MeshBuffer	Buffer{};

	int								SubMeshCount{};
	std::vector<int>				SubSetIndexCounts{};
	std::vector<std::vector<UINT>>	SubSetIndices{};
};
#pragma endregion


#pragma region NameSpace
namespace MeshRenderer {
	void BuildMeshes();

	void Render(const MyBoundingOrientedBox box);
	void Render(const MyBoundingSphere bs);

	void ReleaseStaticUploadBuffers();
	void Release();
}
#pragma endregion


#pragma region Class
class Mesh {
protected:
	UINT mVertexCount{};
	ComPtr<ID3D12Resource> mVertexBuffer{};
	ComPtr<ID3D12Resource> mVertexUploadBuffer{};

	UINT mNormals{};
	ComPtr<ID3D12Resource> mNormalBuffer{};
	ComPtr<ID3D12Resource> mNormalUploadBuffer{};

	UINT mUV0Count{};
	ComPtr<ID3D12Resource> mUV0Buffer{};
	ComPtr<ID3D12Resource> mUV0UploadBuffer{};

	UINT mUV1Count{};
	ComPtr<ID3D12Resource> mUV1Buffer{};
	ComPtr<ID3D12Resource> mUV1UploadBuffer{};

	UINT mTangentCount{};
	ComPtr<ID3D12Resource> mTangentBuffer{};
	ComPtr<ID3D12Resource> mTangentUploadBuffer{};

	UINT mBiTangentCount{};
	ComPtr<ID3D12Resource> mBiTangentBuffer{};
	ComPtr<ID3D12Resource> mBiTangentUploadBuffer{};

	UINT mIndexCount{};
	ComPtr<ID3D12Resource> mIndexBuffer{};
	ComPtr<ID3D12Resource> mIndexUploadBuffer{};

	std::vector<D3D12_VERTEX_BUFFER_VIEW> mVertexBufferViews{};

	D3D12_INDEX_BUFFER_VIEW mIndexBufferView{};
	D3D12_VERTEX_BUFFER_VIEW mNormalBufferView{};

	UINT mSlot{};
	UINT mStride{};
	UINT mOffset{};
	UINT mType{};

public:
	Mesh()          = default;
	virtual ~Mesh() = default;
	
	UINT GetType() const { return mType; }

	std::pair<UINT, UINT> GetLocationInfo() const { return std::make_pair(mIndexCount, mVertexCount); }
	UINT GetIndexCount() const { return mIndexCount; }
	UINT GetVertedxCount() const { return mVertexCount; }

public:
	void ReleaseUploadBuffers();

	virtual void Render() const;
	virtual void RenderInstanced(UINT instanceCount) const;

protected:
	void CreateVertexBufferViews();
	void CreateIndexBuffer(const std::vector<UINT>& indices);
};





class ModelObjectMesh : public Mesh {
public:
	ModelObjectMesh()          = default;
	virtual ~ModelObjectMesh() = default;

public:
	void CreateMeshFromOBB(const BoundingOrientedBox& box);

	void CreateCubeMesh(float width, float height, float depth, bool hasTexture = false, bool isLine = false);

	void CreatePlaneMesh(float width, float depth, bool isLine = false);

	void CreateSphereMesh(float radius, bool isLine = false, int numSegments = 12);
};





class SkyBoxMesh : public Mesh {
public:
	SkyBoxMesh() = default;
	virtual ~SkyBoxMesh() = default;

public:
	SkyBoxMesh(float width, float height, float depth);
	void CreateMesh(float width, float height, float depth);
};





class MergedMesh : public ModelObjectMesh {
private:
	uptr<MeshBuffer> mMeshBuffer{};

	std::vector<ModelInfo> mModelInfo{};

public:
	MergedMesh();
	virtual ~MergedMesh() = default;

	rsptr<Texture> GetTexture() const;

public:
	// 1. MergeMesh
	// 2. MergeMaterial
	// 3. ...
	// 4. SetObject (transform)
	// 5. Close
	bool MergeMesh(rsptr<MeshLoadInfo> mesh, const std::vector<sptr<Material>>& materials);
	void Close();
	virtual void Render(const GameObject* gameObject) const;
	virtual void Render(const ObjectInstanceBuffer* instBuffer) const;
	virtual void RenderSprite(const GameObject* gameObject) const;

	bool HasMesh(UINT index) const { return mModelInfo[index].VertexCount > 0; }

private:
	void MergeSubMeshes(rsptr<MeshLoadInfo> mesh, ModelInfo& modelInfo);

	void Render(const std::vector<const Transform*>& mergedTransform, UINT instanceCount = 1) const;
};
#pragma endregion
