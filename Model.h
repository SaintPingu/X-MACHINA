#pragma once
#include "Component.h"


class CModelObjectMesh;
class CShader;
class CMeshLoadInfo;
class CModelObject;
class CGameObject;
class Camera;

class CTexture;

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
class CMaterialColors
{
public:
	CMaterialColors() { }
	CMaterialColors(const MATERIALLOADINFO& materialInfo);
	virtual ~CMaterialColors() { }

public:
	XMFLOAT4						mAmbient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						mDiffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						mSpecular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); //(r,g,b,a=power)
	XMFLOAT4						mEmissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
};




class CMaterial {
private:
	bool mIsDiffused{ false };
	sptr<CMaterialColors> mMaterialColors{};

public:
	sptr<CTexture> mTexture{};

public:
	CMaterial();
	virtual ~CMaterial();

	void SetMaterialColors(rsptr<CMaterialColors> pMaterialColors) { mMaterialColors = pMaterialColors; }
	void UpdateShaderVariable();
	//void SetTexture(rsptr<CTexture> texture) { mTexture = texture; }
	void LoadTextureFromFile(UINT nType, FILE* file);
	void SetTexture(rsptr<CTexture> texture) { mTexture = texture; }
};










class CMaterial;
class CMasterModel;
class CMergedMesh;


class CModel : public CObject {
private:
	sptr<CMeshLoadInfo> mMeshInfo{};
	std::vector<sptr<CMaterial>> mMaterials{};

	void CopyModelHierarchy(sptr<CModelObject>& object) const;

public:
	CModel();
	~CModel();

	void SetMeshInfo(rsptr<CMeshLoadInfo> meshInfo) { mMeshInfo = meshInfo; }
	void SetMaterials(const std::vector<sptr<CMaterial>>& materials) { mMaterials = materials; }

	// model object의 trasnform 구조를 매핑(Copy)하여 새로운 transform 구조를 반환.
	void CopyModelHierarchy(CGameObject* object) const;

	void MergeModel(CMasterModel& out);
};


class CObjectInstanceBuffer;
class CMasterModel : public CObject {
private:
	sptr<CMergedMesh> mMesh{};

	const CModel* GetModel() const { return mChild->Object<CModel>(); }

public:
	CMasterModel();
	~CMasterModel();

	const std::string& GetName() const { return GetModel()->GetName(); }

	void ReleaseUploadBuffers();
	void MergeMesh(rsptr<CMeshLoadInfo> mesh, const std::vector<sptr<CMaterial>>& materials);
	void Close();

	void Render(const CGameObject* gameObject) const;
	void Render(const CObjectInstanceBuffer* instBuffer = nullptr) const;
private:
	void RenderObject(const CGameObject* gameObject) const;
	void RenderSprite(const CGameObject* gameObject) const;
public:

	Vec4 GetColor() const;

	rsptr<CMergedMesh> GetMesh() const { return mMesh; }
	rsptr<CTexture> GetTexture() const;

	void SetModel(rsptr<CModel> model);
	void SetSprite() { RenderFunc = std::bind(&CMasterModel::RenderSprite, this, std::placeholders::_1); }

	void CopyModelHierarchy(CGameObject* object) const;

private:
	std::function<void(const CGameObject*)> RenderFunc{ std::bind(&CMasterModel::RenderObject, this, std::placeholders::_1) };
};