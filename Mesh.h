//------------------------------------------------------- ----------------------
// File: Mesh.h
//-----------------------------------------------------------------------------

#pragma once
#include "Collider.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
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

	D3D12_PRIMITIVE_TOPOLOGY mPrimitiveTopology{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
	UINT mSlot{};
	UINT mStride{};
	UINT mOffset{};
	UINT mType{};

	UINT mStartIndex{};
	int	mBaseVertex{};

	void CreateVertexBufferViews();

public:
	Mesh() {};
	virtual ~Mesh();
	
	UINT GetType() { return(mType); }

	std::pair<UINT, UINT> GetLocationInfo() const { return std::make_pair(mIndexCount, mVertexCount); }
	UINT GetIndexCount() const { return mIndexCount; }
	UINT GetVertedxCount() const { return mVertexCount; }

	void ReleaseUploadBuffers();

	virtual void Render() const;
	virtual void Render(UINT subset, UINT instanceCount = 1) const;
};


enum class VertexType : DWORD {
	Position = 0x01,
	Color = 0x02,
	Normal = 0x04,
	Tangent = 0x08,
	BiTangent = 0x10,
	UV0 = 0x20,
	UV1 = 0x40,
};

class MeshLoadInfo
{
public:
	MeshLoadInfo() { }
	~MeshLoadInfo();

public:
	std::string mMeshName{};

	DWORD mType = 0x00;

	std::vector<MyBoundingOrientedBox> mOBBList{};
	MyBoundingSphere mBS{};

	int	mVertexCount = 0;
	std::vector<XMFLOAT3> mVertices{};
	std::vector<XMFLOAT3> mNormals{};

	std::vector<XMFLOAT3> mTangents{};
	std::vector<XMFLOAT3> mBiTangents{};
	std::vector<XMFLOAT2> mUV0{};
	std::vector<XMFLOAT2> mUV1{};

	std::vector<UINT> mIndices{};

	int mSubMeshCount{};

	std::vector<int> mSubSetIndexCounts{};
	std::vector<std::vector<UINT>> mSubSetIndices{};
};

class ModelObjectMesh : public Mesh {
public:
	MeshLoadInfo mInfo{};

public:
	ModelObjectMesh();
	virtual ~ModelObjectMesh();

	ModelObjectMesh(const BoundingOrientedBox& box);
	void LoadMeshFromBoundingBox(const BoundingOrientedBox& box);

	ModelObjectMesh(float width, float height, float depth, bool hasTexture = false, bool isLine = false);
	void CreateCubeMesh(float width, float height, float depth, bool hasTexture = false, bool isLine = false);

	ModelObjectMesh(float width, float depth, bool isLine = false);
	void CreatePlaneMesh(float width, float depth, bool isLine = false);

	ModelObjectMesh(float radius, bool isLine = false);
	void CreateSphereMesh(float radius, bool isLine = false);
};


class SkyBoxMesh : public Mesh {
public:
	SkyBoxMesh(float width, float height, float depth);
	void CreateMesh(float width, float height, float depth);
};





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Material;
class Texture;
class Transform;
class ModelObject;
class GameObject;
class ObjectInstanceBuffer;

struct ModelInfo {
	std::vector<UINT> mIndexCounts{};
	UINT mVertexCount{};
	std::vector<sptr<Material>> mMaterials{};
};

struct MeshBuffer {
	std::vector<Vec3> mVertices{};
	std::vector<Vec3> mNormals{};

	std::vector<Vec3> mTangents{};
	std::vector<Vec3> mBiTangents{};
	std::vector<Vec2> mUVs0{};
	std::vector<Vec2> mUVs1{};

	std::vector<UINT> mIndices{};
};

class MergedMesh : public ModelObjectMesh {
private:
	uptr<MeshBuffer> mMeshBuffer{};

	std::vector<ModelInfo> mModelInfo{};

	void MergeSubMeshes(rsptr<MeshLoadInfo> mesh, ModelInfo& modelInfo);

public:
	MergedMesh();

	rsptr<Texture> GetTexture() const;

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

	bool HasMesh(UINT index) const { return mModelInfo[index].mVertexCount > 0; }
};










namespace MeshRenderer {
	void BuildMeshes();

	void Render(const MyBoundingOrientedBox box);
	void Render(const MyBoundingSphere bs);

	void ReleaseStaticUploadBuffers();
	void Release();
}