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
// �޽��� ������ ������ �ִ� ����
enum class VertexType : DWORD {
	Position  = 0x01,
	Color     = 0x02,
	Normal    = 0x04,
	Tangent   = 0x08,
	BiTangent = 0x10,
	UV0       = 0x20,
	UV1       = 0x40,
	Weight    = 0x80,
};
#pragma endregion

#pragma region Struct
// (���� ��������)�� �������� ������ �޽� ����
class SkinMesh;
struct FrameMeshInfo {
	std::vector<sptr<Material>> Materials{};
	std::vector<UINT>			IndicesCnts{};	// sub mesh�� �� indices�� ������, guarantee that Materials.size() == IndicesCnts.size() [�ݵ�� �� �޽����� ������ �־�� �Ѵ�.
	UINT						VertexCnt{};	// ��� mesh�� ���� ����
	sptr<SkinMesh>				SkinMesh{};
};

// sub mesh�� �� �������� �ϳ��� merge�ϱ� ���� �ӽ� ����
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

// file���� ���� (���� ��������)�� �������� mesh ������ ��� ����ü
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

	// static
	void CreateRectangleMesh();
	void CreatePointMesh();
};





// mesh that merged all sub meshes
class MergedMesh : public Mesh {
private:
	uptr<MeshBuffer> mMeshBuffer{};

	std::vector<FrameMeshInfo> mFrameMeshInfo{};	// �� �����Ӹ��� �޽��� ����(����, ��������, ...)�� ������ �����Ѵ�.

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
	// render sprite [object]
	virtual void RenderSprite(const GameObject* object) const;

	bool HasMesh(UINT index) const { return mFrameMeshInfo[index].VertexCnt > 0; }

private:

	// merge all sub meshes from (mesh)
	void MergeSubMeshes(rsptr<MeshLoadInfo> mesh, FrameMeshInfo& modelInfo);

	void Render(const std::vector<const Transform*>& mergedTransform, UINT instanceCnt = 1) const;
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
	std::vector<Transform*>* mBoneFrames{};
	std::vector<BoneType> mBoneTypes{};

private:
	std::vector<Vec4x4> mBoneOffsets{};

public:
	SkinMesh() = default;
	virtual ~SkinMesh() = default;

public:
	BoneType GetBoneType(int boneIndex) const;
	HumanBone GetHumanBone(int boneIndex) const;
	void SetBoneOffsets(const std::vector<Vec4x4>& boneOffsets) { mBoneOffsets = boneOffsets; }

public:
	void UpdateShaderVariables();
};
#pragma endregion
