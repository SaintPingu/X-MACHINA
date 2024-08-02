#pragma once

#pragma region Include
#include "Component/Component.h"
#include "ResourceMgr.h"
#pragma endregion


#pragma region ClassForwardDecl
struct MeshLoadInfo;

class Shader;
class ModelObject;
class GameObject;
class Texture;
class Material;
class MasterModel;
class MergedMesh;
class ObjectPool;
#pragma endregion


#pragma region Struct
// file���� ���� material ������ ��� ����ü
struct MaterialLoadInfo {
	Vec4	DiffuseAlbedo = Vec4::One;
	Vec3	Emission{Vector3::Zero};
	float	Metallic{};
	float	Roughness{};
	bool	OcclusionMask{};
	bool	AlphaTest{};
};

// material�� ����
struct MaterialColors {
public:
	MaterialColors(const MaterialLoadInfo& materialInfo);

	operator const Matrix& () const {
		return *(Matrix*)(this);
	}

public:
	Vec4    DiffuseAlbedo = Vec4::One;
	Vec3	Emission{ Vector3::Zero };
	float	Metallic{};
	float	Roughness{};
	bool	OcclusionMask{};
	bool	AlphaTest{};
};
#pragma endregion



#pragma region Class
class Material {
public:
	std::array<sptr<Texture>, TextureMapCount>	mTextures{}; // ��Ƽ������ ���� ���� �ؽ�ó�� ���� �� �ֵ��� ����
	int	mMatIndex = -1; // ���̴����� �ش� �ε��� ���� ��ȿ���� Ȯ���ϱ� ���ؼ� -1 ���� �ʱⰪ���� �����Ѵ�.

private:
	sptr<MaterialColors> mMaterialColors{};

public:
	Material()          = default;
	virtual ~Material() = default;

	void SetMaterialColors(rsptr<MaterialColors> pMaterialColors) { mMaterialColors = pMaterialColors; }
	void SetTexture(TextureMap map, rsptr<Texture> texture) { assert(texture); mTextures[static_cast<UINT8>(map)] = texture; }
	void SetMatallic(float value) { mMaterialColors->Metallic = value; }
	void SetRoughness(float value) { mMaterialColors->Roughness = value; }

public:
	void UpdateShaderVars();

	void LoadTextureFromFile(TextureMap map, std::ifstream& file);
};




// (�� �������� ������)��ü�� ������ �Ǵ� ��
// mesh�� material ������ ���´�.
class Model : public Object {
private:
	bool mIsSkinMesh{};
	bool mHasMesh{};
	sptr<MeshLoadInfo>			mMeshInfo{};
	std::vector<sptr<Material>> mMaterials{};

public:
	Model()          = default;
	virtual ~Model() = default;

	void SetMeshInfo(rsptr<MeshLoadInfo> meshInfo); 
	void SetMaterials(const std::vector<sptr<Material>>& materials) { mMaterials = materials; }

public:
	// �� Model�� trasnform ���������� [object]�� ���� �����Ѵ�.
	void CopyModelHierarchy(Object* object, Object* parent = nullptr) const;

	// �� Model�� trasnform ���������� ���ϴ� ��� mesh�� material�� ������ [out]���� ��ȯ�Ѵ�.
	void MergeModel(MasterModel& out, bool& isSkinMesh);

public:
	static void MergeTransform(std::vector<Transform*>& out, Model* transform);
};





// ���� ���� ���� ������ �ϳ��� ������ �����ϴ� ��ü
// ���� ��� �޽��� ���� ������ ������ �ִ�.
// �� Ŭ������ ���� ���� ������ ���� ��ü�� ������ �Ѵ�.
struct AnimationLoadInfo;
class MasterModel : public Resource{
private:
	sptr<MergedMesh> mMesh{};
	sptr<AnimationLoadInfo>	mAnimationInfo{};
	sptr<Model> mModel{};
	std::vector<Transform*> mMergedTransform{};

	bool mIsSkinMesh{};		// all mesh is skinned mesh
	bool mMerged{ false };

public:
	MasterModel();
	virtual ~MasterModel() = default;

	rsptr<Model> GetModel() const { return mModel; }
	const Transform* GetTransform() const	{ return mModel.get(); }
	const std::string& GetName() const		{ return mModel->GetName(); }
	rsptr<MergedMesh> GetMesh() const		{ return mMesh; }
	rsptr<Texture> GetTexture() const;

	sptr<const AnimationLoadInfo> GetAnimationInfo() const { return mAnimationInfo; }

	bool IsSkinMesh() const { return mIsSkinMesh; }

	void SetAnimationInfo(sptr<AnimationLoadInfo> animationInfo);

public:
	void ReleaseUploadBuffers();

	// merge [model]'s mesh hierarchy
	void SetModel(const rsptr<Model> model);

	// merge [mesh] to [mMesh], call MergedMesh::MergeMesh().
	void MergeMesh(sptr<MeshLoadInfo>& mesh, std::vector<sptr<Material>>& materials) const;

	// render single [object]
	void Render(const GameObject* object) const;
	// render instancing objects from [objectPool]
	void Render(const ObjectPool* objectPool = nullptr) const;

	// Model�� trasnform ���������� [object]�� �����Ѵ�.
	// call Model::CopyModelHierarchy()
	void CopyModelHierarchy(GameObject* object) const;

	std::vector<Transform*> GetMergedTransform() const { return mMergedTransform; }

private:
	void MergeModelTransforms();
};
#pragma endregion
