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
// file���� ���� material ������ ��� ����ü
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

// material�� ����
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




// (�� ��������)��ü�� ������ �Ǵ� ��
// mesh�� material ������ ���´�.
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
	// �� Model�� trasnform ���������� [object]�� �����Ѵ�.
	void CopyModelHierarchy(GameObject* object) const;

	// �� Model�� trasnform ���������� ���ϴ� ��� mesh�� material�� ������ [out]���� ��ȯ�Ѵ�.
	void MergeModel(MasterModel& out);

private:
	// [object]�� �����ϸ�, �� Model�� trasnform ������ [object]�� �����Ѵ�.
	void CopyModelHierarchy(sptr<Object>& object) const;
};





// ���� ���� ���� ������ �ϳ��� ������ �����ϴ� ��ü
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

	// ���� ��������Ʈ�� �����Ѵ�.
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

	// Model�� trasnform ���������� [object]�� �����Ѵ�.
	// call Model::CopyModelHierarchy()
	void CopyModelHierarchy(GameObject* object) const;

private:
	std::function<void(const GameObject*)> RenderFunc{ std::bind(&MasterModel::RenderObject, this, std::placeholders::_1) };	// ������ �Լ�

	// ���� ��ü ������
	void RenderObject(const GameObject* gameObject) const;

	// ��������Ʈ ��ü ������
	void RenderSprite(const GameObject* gameObject) const;
};
#pragma endregion
