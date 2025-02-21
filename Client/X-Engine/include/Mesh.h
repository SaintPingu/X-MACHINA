#pragma once
#include "HumanBone.h"

#pragma region Include
#include "Resources.h"
#pragma endregion

#pragma region ClassForwardDecl
class Material;
class Texture;
class Transform;
class ModelObject;
class GameObject;
class ObjectPool;
class Object;
#pragma endregion


#pragma region EnumClass
// 메쉬의 정점이 가지고 있는 정보
class VertexType : public DwordOverloader<VertexType> {
	DWORD_OVERLOADER(VertexType)

	static const DWORD Position  = 0x01;
	static const DWORD Color     = 0x02;
	static const DWORD Normal    = 0x04;
	static const DWORD Tangent   = 0x08;
	static const DWORD BiTangent = 0x10;
	static const DWORD UV0       = 0x20;
	static const DWORD UV1       = 0x40;
	static const DWORD Weight    = 0x80;
};
#pragma endregion

#pragma region Struct
// (계층 구조에서)한 프레임이 가지는 메쉬 정보
class SkinMesh;
struct FrameMeshInfo {
	std::vector<sptr<Material>> Materials{};
	std::vector<UINT>			IndicesCnts{};	// sub mesh의 각 indices의 개수들, guarantee that Materials.size() == IndicesCnts.size() [반드시 각 메쉬마다 재질이 있어야 한다.
	UINT						VertexCnt{};	// 모든 mesh의 정점 개수
	sptr<SkinMesh>				SkinMesh{};
};

// sub mesh의 각 정보들을 하나로 merge하기 위한 임시 버퍼
struct MeshBuffer {
	std::vector<Vec3> Vertices{};
	std::vector<Vec3> Normals{};
	std::vector<Vec3> Tangents{};
	std::vector<Vec3> BiTangents{};
	std::vector<Vec2> UVs0{};
	std::vector<Vec2> UVs1{};
	std::vector<XMINT4> BoneIndices;
	std::vector<Vec4> BoneWeights;

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

	sptr<SkinMesh>	SkinMesh{};
};
#pragma endregion



#pragma region Class
class Mesh : public Resource {
protected:
	UINT mVertexCnt{};
	ComPtr<ID3D12Resource> mVertexBuffer{};
	ComPtr<ID3D12Resource> mVertexUploadBuffer{};

	ComPtr<ID3D12Resource> mNormalBuffer{};
	ComPtr<ID3D12Resource> mNormalUploadBuffer{};

	ComPtr<ID3D12Resource> mUV0Buffer{};
	ComPtr<ID3D12Resource> mUV0UploadBuffer{};

	ComPtr<ID3D12Resource> mUV1Buffer{};
	ComPtr<ID3D12Resource> mUV1UploadBuffer{};

	ComPtr<ID3D12Resource> mTangentBuffer{};
	ComPtr<ID3D12Resource> mTangentUploadBuffer{};

	ComPtr<ID3D12Resource> mBiTangentBuffer{};
	ComPtr<ID3D12Resource> mBiTangentUploadBuffer{};

	ComPtr<ID3D12Resource> mBoneIndexBuffer{};
	ComPtr<ID3D12Resource> mBoneIndexUploadBuffer{};

	ComPtr<ID3D12Resource> mBoneWeightBuffer{};
	ComPtr<ID3D12Resource> mBoneWeightUploadBuffer{};

	UINT mIndexCnt{};
	ComPtr<ID3D12Resource> mIndexBuffer{};
	ComPtr<ID3D12Resource> mIndexUploadBuffer{};

	std::vector<D3D12_VERTEX_BUFFER_VIEW>	mVertexBufferViews{};
	D3D12_INDEX_BUFFER_VIEW					mIndexBufferView{};

	static constexpr UINT mSlot   = 0;	// StartSlot
	static constexpr UINT mOffset = 0;	// StartVertexLocation

public:
	Mesh() : Resource(ResourceType::Mesh) { }
	virtual ~Mesh() = default;

public:
	void ReleaseUploadBuffers();

	virtual void Render() const;
	virtual void RenderInstanced(UINT instanceCnt) const;

protected:
	void CreateVertexBufferViews();
	void CreateIndexBufferView(const std::vector<UINT>& indices);
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
	void CreatePlaneMesh(float width, float depth, bool isLine = false);
	void CreateSphereMesh(float radius, int numSegments = 12, bool isLine = false);
	void CreateSphere(float radius, int sliceCount, int stackCount);

	// static
	void CreateRectangleMesh();
	void CreatePointMesh();
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

	// update frame resource of material buffer
	void UpdateMaterialBuffer();

	// render [object] for this mesh
	virtual void Render(const GameObject* object) const;
	// render instanced objects from [objectPool]
	virtual void Render(const ObjectPool* objectPool) const;

	bool HasMesh(UINT index) const { return mFrameMeshInfo[index].VertexCnt > 0; }

private:

	// merge all sub meshes from (mesh)
	void MergeSubMeshes(rsptr<MeshLoadInfo> mesh, FrameMeshInfo& modelInfo);

	void Render(const std::vector<Transform*>& mergedTransform, UINT instanceCnt = 1) const;
};

class Avatar {
private:
	std::unordered_map<std::string, BoneType> mBoneTypes{};

public:
	Avatar() = default;
	virtual ~Avatar() = default;

public:
	BoneType GetBoneType(const std::string& boneName) const;
	void SetBoneType(const std::string& boneName, const std::string& boneType);
};

class SkinMesh : public Mesh {
public:
	std::vector<std::string> mBoneNames;
	std::unordered_map<std::string, int> mBoneNameIndices;
	std::unordered_map<std::string, Transform*>* mBoneFrames{};
	std::unordered_map<std::string, BoneType> mBoneTypes{};

private:
	std::vector<Matrix> mBoneOffsets{};

public:
	SkinMesh() = default;
	virtual ~SkinMesh() = default;

public:
	BoneType GetBoneType(const std::string& boneName) const;
	HumanBone GetHumanBone(const std::string& boneName) const;
	void SetBoneOffsets(const std::vector<Matrix>& boneOffsets) { mBoneOffsets = boneOffsets; }

public:
	void UpdateShaderVariables();
};
#pragma endregion
