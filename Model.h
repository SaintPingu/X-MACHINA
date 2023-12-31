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
struct MaterialLoadInfo
{
	Vec4 Albedo   = Vec4(1.f, 1.f, 1.f, 1.f);
	Vec4 Emissive = Vec4(0.f, 0.f, 0.f, 1.f);
	Vec4 Specular = Vec4(0.f, 0.f, 0.f, 1.f);

	float Glossiness = 0.f;
	float Smoothness = 0.f;
	float SpecularHighlight = 0.f;
	float Metallic = 0.f;
	float GlossyReflection = 0.f;

	UINT Type = 0x00;
};
#pragma endregion


#pragma region Class
class MaterialColors
{
public:
	MaterialColors()  = default;
	MaterialColors(const MaterialLoadInfo& materialInfo);
	~MaterialColors() = default;

public:
	Vec4 mAmbient = Vec4(1.f, 1.f, 1.f, 1.f);
	Vec4 mDiffuse = Vec4(0.f, 0.f, 0.f, 1.f);
	Vec4 mSpecular = Vec4(0.f, 0.f, 0.f, 1.f); //(r,g,b,a=power)
	Vec4 mEmissive = Vec4(0.f, 0.f, 0.f, 1.f);
};





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





class Model : public Object {
private:
	sptr<MeshLoadInfo> mMeshInfo{};
	std::vector<sptr<Material>> mMaterials{};

public:
	Model()          = default;
	virtual ~Model() = default;

	void SetMeshInfo(rsptr<MeshLoadInfo> meshInfo) { mMeshInfo = meshInfo; }
	void SetMaterials(const std::vector<sptr<Material>>& materials) { mMaterials = materials; }

public:
	// model object의 trasnform 구조를 매핑(Copy)하여 새로운 transform 구조를 반환.
	void CopyModelHierarchy(GameObject* object) const;

	void MergeModel(MasterModel& out);

private:
	void CopyModelHierarchy(sptr<ModelObject>& object) const;
};





class MasterModel : public Object {
private:
	sptr<MergedMesh> mMesh{};

public:
	MasterModel();
	virtual ~MasterModel() = default;

	const std::string& GetName() const { return GetModel()->GetName(); }
	rsptr<MergedMesh> GetMesh() const { return mMesh; }
	rsptr<Texture> GetTexture() const;

	void SetModel(rsptr<Model> model);
	void SetSprite() { RenderFunc = std::bind(&MasterModel::RenderSprite, this, std::placeholders::_1); }

public:
	void ReleaseUploadBuffers();
	void MergeMesh(rsptr<MeshLoadInfo> mesh, const std::vector<sptr<Material>>& materials);
	void Close();

	void Render(const GameObject* gameObject) const { RenderFunc(gameObject); }
	void Render(const ObjectInstanceBuffer* instBuffer = nullptr) const;

	void CopyModelHierarchy(GameObject* object) const;

private:
	std::function<void(const GameObject*)> RenderFunc{ std::bind(&MasterModel::RenderObject, this, std::placeholders::_1) };
	const Model* GetModel() const { return mChild->GetObj<Model>(); }

	void RenderObject(const GameObject* gameObject) const;
	void RenderSprite(const GameObject* gameObject) const;
};
#pragma endregion
