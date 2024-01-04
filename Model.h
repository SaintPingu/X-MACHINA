#pragma once

#pragma region Include
#include "Component.h"
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
	Vec4 Albedo = Vector4::One();
	Vec4 Emissive{};
	Vec4 Specular{};

	float Glossiness{};
	float Smoothness{};
	float SpecularHighlight{};
	float Metallic{};
	float GlossyReflection{};
};

// material의 색상
struct MaterialColors {
public:
	MaterialColors(const MaterialLoadInfo& materialInfo);

	operator const Vec4x4& () const {
		return *(Vec4x4*)(this);
	}

public:
	Vec4 Ambient{ Vector4::One() };
	Vec4 Diffuse{};
	Vec4 Specular{};
	Vec4 Emissive{};
};

#pragma endregion



#pragma region Class
class Material {
public:
	sptr<Texture> mTexture{};

private:
	sptr<MaterialColors> mMaterialColors{};

public:
	Material()          = default;
	virtual ~Material() = default;

	void SetMaterialColors(rsptr<MaterialColors> pMaterialColors) { mMaterialColors = pMaterialColors; }
	void SetTexture(rsptr<Texture> texture) { mTexture = texture; }

public:
	void UpdateShaderVars();
	void LoadTextureFromFile(FILE* file);
};




// (각 계층구조)객체의 원본이 되는 모델
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
class MasterModel {
private:
	sptr<MergedMesh> mMesh{};
	sptr<Model> mModel{};

	bool mMerged{ false };

public:
	MasterModel();
	virtual ~MasterModel() = default;

	const Transform* GetTransform() const { return mModel.get(); }
	const std::string& GetName() const { return mModel->GetName(); }
	rsptr<MergedMesh> GetMesh() const { return mMesh; }
	rsptr<Texture> GetTexture() const;

	// 모델을 스프라이트로 설정한다.
	void SetSprite() { RenderFunc = std::bind(&MasterModel::RenderSprite, this, std::placeholders::_1); }

public:
	void ReleaseUploadBuffers();

	// merge [model]'s mesh hierarchy
	void SetModel(const rsptr<Model> model);

	// merge [mesh] to [mMesh], call MergedMesh::MergeMesh().
	void MergeMesh(sptr<MeshLoadInfo>& mesh, std::vector<sptr<Material>>& materials);

	// render single [object]
	void Render(const GameObject* object) const { RenderFunc(object); }
	// render instancing objects from [instBuffer]
	void Render(const ObjectPool* instBuffer = nullptr) const;

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
