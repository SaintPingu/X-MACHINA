#pragma once



#pragma region ClassForwardDecl
class Material;
class Texture;
class Transform;
class ModelObject;
class GameObject;
class ObjectPool;
#pragma endregion


#pragma region EnumClass
// 메쉬의 정점이 가지고 있는 정보
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
// (계층 구조에서)한 프레임이 가지는 메쉬 정보
struct FrameMeshInfo {
	std::vector<sptr<Material>> Materials{};
	std::vector<UINT>			IndicesCnts{};	// sub mesh의 각 indices의 개수들, guarantee that Materials.size() == IndicesCnts.size() [반드시 각 메쉬마다 재질이 있어야 한다.
	UINT						VertexCnt{};	// 모든 mesh의 정점 개수
};

// sub mesh의 각 정보들을 하나로 merge하기 위한 임시 버퍼
struct MeshBuffer {
	std::vector<Vec3> Vertices{};
	std::vector<Vec3> Normals{};
	std::vector<Vec3> Tangents{};
	std::vector<Vec3> BiTangents{};
	std::vector<Vec2> UVs0{};
	std::vector<Vec2> UVs1{};

	std::vector<UINT> Indices{};
};

// file에서 읽은 (계층 구조에서)한 프레임의 mesh 정보를 담는 구조체
struct MeshLoadInfo {
	DWORD		VertexType{};
	std::string MeshName{};

	int			VertexCnt{};
	MeshBuffer	Buffer{};

	int								SubMeshCnt{};
	std::vector<int>				SubSetIndexCnts{};
	std::vector<std::vector<UINT>>	SubSetIndices{};
};
#pragma endregion



#pragma region Class
class Mesh {
protected:
	UINT mVertexCnt{};
	ComPtr<ID3D12Resource> mVertexBuffer{};
	ComPtr<ID3D12Resource> mVertexUploadBuffer{};

	UINT mNormalCnt{};
	ComPtr<ID3D12Resource> mNormalBuffer{};
	ComPtr<ID3D12Resource> mNormalUploadBuffer{};

	UINT mUV0Cnt{};
	ComPtr<ID3D12Resource> mUV0Buffer{};
	ComPtr<ID3D12Resource> mUV0UploadBuffer{};

	UINT mUV1Cnt{};
	ComPtr<ID3D12Resource> mUV1Buffer{};
	ComPtr<ID3D12Resource> mUV1UploadBuffer{};

	UINT mTangentCnt{};
	ComPtr<ID3D12Resource> mTangentBuffer{};
	ComPtr<ID3D12Resource> mTangentUploadBuffer{};

	UINT mBiTangentCnt{};
	ComPtr<ID3D12Resource> mBiTangentBuffer{};
	ComPtr<ID3D12Resource> mBiTangentUploadBuffer{};

	UINT mIndexCnt{};
	ComPtr<ID3D12Resource> mIndexBuffer{};
	ComPtr<ID3D12Resource> mIndexUploadBuffer{};

	std::vector<D3D12_VERTEX_BUFFER_VIEW>	mVertexBufferViews{};
	D3D12_INDEX_BUFFER_VIEW					mIndexBufferView{};

	static constexpr UINT mSlot   = 0;	// StartSlot
	static constexpr UINT mOffset = 0;	// StartVertexLocation

public:
	Mesh()          = default;
	virtual ~Mesh() = default;

public:
	void ReleaseUploadBuffers();

	virtual void Render() const;
	virtual void RenderInstanced(UINT instanceCnt) const;

protected:
	void CreateVertexBufferViews();
	void CreateIndexBuffer(const std::vector<UINT>& indices);
};




// basic shape mesh (cube, plane, sphere, ...)
class ModelObjectMesh : public Mesh {
public:
	ModelObjectMesh()          = default;
	virtual ~ModelObjectMesh() = default;

public:
	void CreateMeshFromOBB(const BoundingOrientedBox& box);

	// isLine => D3D_PRIMITIVE_TOPOLOGY_LINELIST
	void CreateCubeMesh(float width, float height, float depth, bool hasTexture = false, bool isLine = false);
	void CreateSkyBoxMesh(float width, float height, float depth);

	void CreatePlaneMesh(float width, float depth, bool isLine = false);

	void CreateSphereMesh(float radius, int numSegments = 12, bool isLine = false);

};





// mesh that merged all sub meshes
class MergedMesh : public Mesh {
private:
	uptr<MeshBuffer> mMeshBuffer{};

	std::vector<FrameMeshInfo> mFrameMeshInfo{};	// 각 프레임마다 메쉬의 정보(재질, 정점개수, ...)를 별도로 저장한다.

public:
	MergedMesh();
	virtual ~MergedMesh() = default;

	// return the first texture of meshes
	rsptr<Texture> GetTexture() const;

public:
	
	// merge all meshes to [mFrameMeshInfo] from [mesh] and [materials]
	void MergeMesh(sptr<MeshLoadInfo>& mesh, std::vector<sptr<Material>>& materials);
	// stop merge and create buffer resource
	void StopMerge();

	// render [object] for this mesh
	virtual void Render(const GameObject* object) const;
	// render instanced objects from [objectPool]
	virtual void Render(const ObjectPool* objectPool) const;
	// render sprite [object]
	virtual void RenderSprite(const GameObject* object) const;

	bool HasMesh(UINT index) const { return mFrameMeshInfo[index].VertexCnt > 0; }

private:

	// merge all sub meshes from (mesh)
	void MergeSubMeshes(rsptr<MeshLoadInfo> mesh, FrameMeshInfo& modelInfo);

	void Render(const std::vector<const Transform*>& mergedTransform, UINT instanceCnt = 1) const;
};
#pragma endregion
