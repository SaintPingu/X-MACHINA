#pragma once

#pragma region Include
#include "Component.h"
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
// file에서 읽은 material 정보를 담는 구조체
struct MaterialLoadInfo {
	Vec4	DiffuseAlbedo = Vector4::One();
	float	Metallic{};
	float	Roughness{};
};

// material의 색상
struct MaterialColors {
public:
	MaterialColors(const MaterialLoadInfo& materialInfo);

	operator const Vec4x4& () const {
		return *(Vec4x4*)(this);
	}

public:
	Vec4    DiffuseAlbedo = Vector4::One();
	float	Metallic{};
	float	Roughness{};
};
#pragma endregion



#pragma region Class
class Material {
public:
	std::array<sptr<Texture>, TextureMapCount>	mTextures{}; // 머티리얼이 여러 개의 텍스처를 가질 수 있도록 변경
	int	mMatIndex = -1; // 쉐이더에서 해당 인덱스 값이 유효한지 확인하기 위해서 -1 값을 초기값으로 설정한다.

private:
	sptr<MaterialColors> mMaterialColors{};

public:
	Material()          = default;
	virtual ~Material() = default;

	void SetMaterialColors(rsptr<MaterialColors> pMaterialColors) { mMaterialColors = pMaterialColors; }
	void SetTexture(TextureMap map, rsptr<Texture> texture) { mTextures[static_cast<UINT8>(map)] = texture; }
	void SetMatallic(float value) { mMaterialColors->Metallic = value; }
	void SetRoughness(float value) { mMaterialColors->Roughness = value; }

public:
	void UpdateShaderVars();

	void LoadTextureFromFile(TextureMap map, FILE* file);
};




// (각 계층구조 프레임)객체의 원본이 되는 모델
// mesh와 material 정보를 갖는다.
class Model : public Object {
private:
	sptr<MeshLoadInfo>			mMeshInfo{};
	std::vector<sptr<Material>> mMaterials{};

public:
	Model()          = default;
	virtual ~Model() = default;

	void SetMeshInfo(rsptr<MeshLoadInfo> meshInfo) { mMeshInfo = meshInfo; }
	void SetMaterials(const std::vector<sptr<Material>>& materials) { mMaterials = materials; }

public:
	// 이 Model의 trasnform 계층구조를 [object]에 복사 생성한다.
	void CopyModelHierarchy(Object* object) const;

	// 이 Model의 trasnform 계층구조에 속하는 모든 mesh와 material을 병합해 [out]으로 반환한다.
	void MergeModel(MasterModel& out); 
};





// 모델의 계층 구조 정보를 하나로 병합해 관리하는 객체
// 모델의 모든 메쉬와 재질 정보를 가지고 있다.
// 이 클래스를 통해 모델을 가지는 게임 객체를 렌더링 한다.
struct AnimationLoadInfo;
class MasterModel : public Resource{
private:
	sptr<MergedMesh> mMesh{};
	sptr<AnimationLoadInfo>	mAnimationInfo{};
	sptr<Model> mModel{};

	bool mMerged{ false };

public:
	MasterModel();
	virtual ~MasterModel() = default;

	const Transform* GetTransform() const	{ return mModel.get(); }
	const std::string& GetName() const		{ return mModel->GetName(); }
	rsptr<MergedMesh> GetMesh() const		{ return mMesh; }
	rsptr<Texture> GetTexture() const;

	// 모델을 스프라이트로 설정한다.
	void SetSprite() { RenderFunc = std::bind(&MasterModel::RenderSprite, this, std::placeholders::_1); }

	sptr<const AnimationLoadInfo> GetAnimationInfo() const { return mAnimationInfo; }
	void SetAnimationInfo(sptr<AnimationLoadInfo> animationInfo);

public:
	void ReleaseUploadBuffers();

	// merge [model]'s mesh hierarchy
	void SetModel(const rsptr<Model> model);

	// merge [mesh] to [mMesh], call MergedMesh::MergeMesh().
	void MergeMesh(sptr<MeshLoadInfo>& mesh, std::vector<sptr<Material>>& materials) const;

	// render single [object]
	void Render(const GameObject* object) const { RenderFunc(object); }
	// render instancing objects from [objectPool]
	void Render(const ObjectPool* objectPool = nullptr) const;

	// Model의 trasnform 계층구조를 [object]에 복사한다.
	// call Model::CopyModelHierarchy()
	void CopyModelHierarchy(GameObject* object) const;

private:
	std::function<void(const GameObject*)> RenderFunc{ std::bind(&MasterModel::RenderObject, this, std::placeholders::_1) };	// 렌더링 함수

	// render [object]
	void RenderObject(const GameObject* object) const;

	// render sprite [object]
	void RenderSprite(const GameObject* object) const;
};
#pragma endregion
