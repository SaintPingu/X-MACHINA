#pragma once
#include "Component.h"

//-----------------------------[Class Declaration]-----------------------------//
class ModelObjectMesh;
class Shader;
class MeshLoadInfo;
class ModelObject;
class GameObject;
class Camera;
class Texture;
//-----------------------------------------------------------------------------//

// Constant Buffers
struct CB_COLOR_OBJECT_INFO
{
	Vec4x4 mWorld{};
	Vec3 mColor{};
};

struct CB_MATERIAL_OBJECT_INFO
{
	Vec4x4 mWorld{};
	UINT mMaterial{};
};


// Structures
struct MATERIALLOADINFO
{
	XMFLOAT4						mAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						mEmissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						mSpecular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	float							mGlossiness = 0.0f;
	float							mSmoothness = 0.0f;
	float							mSpecularHighlight = 0.0f;
	float							mMetallic = 0.0f;
	float							mGlossyReflection = 0.0f;

	UINT							mType = 0x00;
};

// Classes
class MaterialColors
{
public:
	MaterialColors() { }
	MaterialColors(const MATERIALLOADINFO& materialInfo);
	virtual ~MaterialColors() { }

public:
	XMFLOAT4						mAmbient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						mDiffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						mSpecular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); //(r,g,b,a=power)
	XMFLOAT4						mEmissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
};




class Material {
private:
	bool mIsDiffused{ false };
	sptr<MaterialColors> mMaterialColors{};

public:
	sptr<Texture> mTexture{};

public:
	Material();
	virtual ~Material();

	void SetMaterialColors(rsptr<MaterialColors> pMaterialColors) { mMaterialColors = pMaterialColors; }
	void UpdateShaderVariable();
	void LoadTextureFromFile(UINT nType, FILE* file);
	void SetTexture(rsptr<Texture> texture) { mTexture = texture; }
};










class Material;
class MasterModel;
class MergedMesh;


class Model : public Object {
private:
	sptr<MeshLoadInfo> mMeshInfo{};
	std::vector<sptr<Material>> mMaterials{};

	void CopyModelHierarchy(sptr<ModelObject>& object) const;

public:
	Model();
	~Model();

	void SetMeshInfo(rsptr<MeshLoadInfo> meshInfo) { mMeshInfo = meshInfo; }
	void SetMaterials(const std::vector<sptr<Material>>& materials) { mMaterials = materials; }

	// model object의 trasnform 구조를 매핑(Copy)하여 새로운 transform 구조를 반환.
	void CopyModelHierarchy(GameObject* object) const;

	void MergeModel(MasterModel& out);
};


class ObjectInstanceBuffer;
class MasterModel : public Object {
private:
	sptr<MergedMesh> mMesh{};

	const Model* GetModel() const { return mChild->GetObj<Model>(); }

public:
	MasterModel();
	~MasterModel();

	const std::string& GetName() const { return GetModel()->GetName(); }

	void ReleaseUploadBuffers();
	void MergeMesh(rsptr<MeshLoadInfo> mesh, const std::vector<sptr<Material>>& materials);
	void Close();

	void Render(const GameObject* gameObject) const { RenderFunc(gameObject); }
	void Render(const ObjectInstanceBuffer* instBuffer = nullptr) const;
private:
	void RenderObject(const GameObject* gameObject) const;
	void RenderSprite(const GameObject* gameObject) const;
public:

	rsptr<MergedMesh> GetMesh() const { return mMesh; }
	rsptr<Texture> GetTexture() const;

	void SetModel(rsptr<Model> model);
	void SetSprite() { RenderFunc = std::bind(&MasterModel::RenderSprite, this, std::placeholders::_1); }

	void CopyModelHierarchy(GameObject* object) const;

private:
	std::function<void(const GameObject*)> RenderFunc{ std::bind(&MasterModel::RenderObject, this, std::placeholders::_1) };
};