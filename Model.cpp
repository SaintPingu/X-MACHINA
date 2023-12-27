#include "stdafx.h"
#include "Model.h"
#include "DXGIMgr.h"

#include "Object.h"
#include "Camera.h"
#include "Mesh.h"
#include "Collider.h"
#include "Texture.h"
#include "Scene.h"
#include "Shader.h"











// [ CModel ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CModel::CModel()
{
	
}
CModel::~CModel()
{

}

void CModel::CopyModelHierarchy(sptr<CModelObject>& object) const
{
	object = std::make_shared<CModelObject>();

	object->CopyComponents(*this);
	object->SetTransform(GetLocalTransform());
	object->SetName(mName);


	if (mSibling) {
		sptr<CModelObject> sibling{};
		mSibling->Object<CModel>()->CopyModelHierarchy(sibling);
		object->mSibling = sibling;
	}
	if (mChild) {
		sptr<CModelObject> child{};
		mChild->Object<CModel>()->CopyModelHierarchy(child);
		object->SetChild(child);
	}
}

void CModel::CopyModelHierarchy(CGameObject* object) const
{
	object->CopyComponents(*this);
	object->SetTransform(GetLocalTransform());
	object->SetName(mName);


	if (mSibling) {
		sptr<CModelObject> sibling{};
		mSibling->Object<CModel>()->CopyModelHierarchy(sibling);
		object->mSibling = sibling;
	}
	if (mChild) {
		sptr<CModelObject> child{};
		mChild->Object<CModel>()->CopyModelHierarchy(child);
		object->SetChild(child);
	}
}

void CModel::MergeModel(CMasterModel& out)
{
	out.MergeMesh(mMeshInfo, mMaterials);
	mMeshInfo = nullptr;
	mMaterials.clear();

	if (mSibling) {
		mSibling->Object<CModel>()->MergeModel(out);
	}
	if (mChild) {
		mChild->Object<CModel>()->MergeModel(out);
	}
}








// [ CMaterial ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMaterialColors::CMaterialColors(const MATERIALLOADINFO& materialInfo)
{
	mDiffuse = Vector4::Normalize(materialInfo.mAlbedo);
	mSpecular = materialInfo.mSpecular; //(r,g,b,a=power)
	mSpecular.w = (materialInfo.mGlossiness * 255.0f);
	mEmissive = materialInfo.mEmissive;
}

CMaterial::CMaterial()
{
}

CMaterial::~CMaterial()
{
}

void CMaterial::UpdateShaderVariable()
{
	constexpr RootParam param = RootParam::GameObjectInfo;

	if (mTexture) {
		mTexture->UpdateShaderVariables();
	}
	else {
		UINT mask = 0x00;
		crntScene->SetGraphicsRoot32BitConstants(param, mask, 32);
	}

	if (!mMaterialColors) {
		Vec4 ambient{}, diffuse{}, specular{}, emmisive{};

		crntScene->SetGraphicsRoot32BitConstants(param, ambient, 16);
		crntScene->SetGraphicsRoot32BitConstants(param, diffuse, 20);
		crntScene->SetGraphicsRoot32BitConstants(param, specular, 24);
		crntScene->SetGraphicsRoot32BitConstants(param, emmisive, 28);

		return;
	}

	crntScene->SetGraphicsRoot32BitConstants(param, mMaterialColors->mDiffuse, 20);

	if (mIsDiffused) {
		return;
	}

	crntScene->SetGraphicsRoot32BitConstants(param, mMaterialColors->mAmbient, 16);
	crntScene->SetGraphicsRoot32BitConstants(param, mMaterialColors->mSpecular, 24);
	crntScene->SetGraphicsRoot32BitConstants(param, mMaterialColors->mEmissive, 28);
}

void CMaterial::LoadTextureFromFile(UINT nType, FILE* file)
{
	std::string textureName{};
	::ReadStringFromFile(file, textureName);
	
	if (textureName == "null") {
		return;
	}

	mTexture = crntScene->GetTexture(textureName);
}



// [ CMasterModel ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMasterModel::CMasterModel()
{
	mMesh = std::make_shared<CMergedMesh>();
}


CMasterModel::~CMasterModel()
{
	
}

void CMasterModel::ReleaseUploadBuffers()
{
	mMesh->ReleaseUploadBuffers();
}

Vec4 CMasterModel::GetColor() const
{
	return Vec4(1.0f, 1.0f, 0.0f, 1.0f);
}

rsptr<CTexture> CMasterModel::GetTexture() const
{
	return mMesh->GetTexture();
}

void CMasterModel::SetModel(rsptr<CModel> model)
{
	SetChild(model);
}

void CMasterModel::MergeMesh(rsptr<CMeshLoadInfo> mesh, const std::vector<sptr<CMaterial>>& materials)
{
	mMesh->MergeMesh(mesh, materials);
}


void CMasterModel::Close()
{
	mMesh->Close();
}

void CMasterModel::Render(const CGameObject* gameObject) const
{
	RenderFunc(gameObject);
}

void CMasterModel::Render(const CObjectInstanceBuffer* instBuffer) const
{
	mMesh->Render(instBuffer);
}

void CMasterModel::RenderObject(const CGameObject* gameObject) const
{
	mMesh->Render(gameObject);
}

void CMasterModel::RenderSprite(const CGameObject* gameObject) const
{
	mMesh->RenderSprite(gameObject);
}

void CMasterModel::CopyModelHierarchy(CGameObject* object) const
{
	GetModel()->CopyModelHierarchy(object);
}