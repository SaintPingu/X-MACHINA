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
class ObjectInstanceBuffer;
#pragma endregion


#pragma region Struct
// file에서 읽은 material 정보를 담는 구조체
struct MaterialLoadInfo {
	Vec4 Albedo{ Vector4::One() };
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

public:
	Vec4 Ambient{};
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
	bool mIsDiffused{ false };
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
	// 이 Model의 trasnform 계층구조를 [object]에 복사한다.
	void CopyModelHierarchy(GameObject* object) const;

	// 이 Model의 trasnform 계층구조에 속하는 모든 mesh와 material을 병합해 [out]으로 반환한다.
	void MergeModel(MasterModel& out);

private:
	// [object]를 생성하며, 이 Model의 trasnform 구조를 [object]에 복사한다.
	void CopyModelHierarchy(sptr<Object>& object) const;
};





// 모델의 계층 구조 정보를 하나로 병합해 관리하는 객체
class MasterModel {
private:
	sptr<MergedMesh> mMesh{};
	sptr<Model> mModel{};

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

	// merge model's mesh hierarchy
	void SetModel(const rsptr<Model> model);

	// merge [mesh] to [mMesh], call MergedMesh::MergeMesh().
	void MergeMesh(sptr<MeshLoadInfo>& mesh, std::vector<sptr<Material>>& materials);

	// render single object
	void Render(const GameObject* gameObject) const { RenderFunc(gameObject); }

	// render instancing object from instance buffer
	void Render(const ObjectInstanceBuffer* instBuffer = nullptr) const;

	// Model의 trasnform 계층구조를 [object]에 복사한다.
	// call Model::CopyModelHierarchy()
	void CopyModelHierarchy(GameObject* object) const;

private:
	std::function<void(const GameObject*)> RenderFunc{ std::bind(&MasterModel::RenderObject, this, std::placeholders::_1) };	// 렌더링 함수

	// 단일 객체 렌더링
	void RenderObject(const GameObject* gameObject) const;

	// 스프라이트 객체 렌더링
	void RenderSprite(const GameObject* gameObject) const;
};
#pragma endregion
