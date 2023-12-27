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











// [ Model ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Model::Model()
{
	
}
Model::~Model()
{

}

void Model::CopyModelHierarchy(sptr<ModelObject>& object) const
{
	object = std::make_shared<ModelObject>();

	object->CopyComponents(*this);
	object->SetTransform(GetLocalTransform());
	object->SetName(mName);


	if (mSibling) {
		sptr<ModelObject> sibling{};
		mSibling->GetObj<Model>()->CopyModelHierarchy(sibling);
		object->mSibling = sibling;
	}
	if (mChild) {
		sptr<ModelObject> child{};
		mChild->GetObj<Model>()->CopyModelHierarchy(child);
		object->SetChild(child);
	}
}

void Model::CopyModelHierarchy(GameObject* object) const
{
	object->CopyComponents(*this);
	object->SetTransform(GetLocalTransform());
	object->SetName(mName);


	if (mSibling) {
		sptr<ModelObject> sibling{};
		mSibling->GetObj<Model>()->CopyModelHierarchy(sibling);
		object->mSibling = sibling;
	}
	if (mChild) {
		sptr<ModelObject> child{};
		mChild->GetObj<Model>()->CopyModelHierarchy(child);
		object->SetChild(child);
	}
}

void Model::MergeModel(MasterModel& out)
{
	out.MergeMesh(mMeshInfo, mMaterials);
	mMeshInfo = nullptr;
	mMaterials.clear();

	if (mSibling) {
		mSibling->GetObj<Model>()->MergeModel(out);
	}
	if (mChild) {
		mChild->GetObj<Model>()->MergeModel(out);
	}
}








// [ Material ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MaterialColors::MaterialColors(const MATERIALLOADINFO& materialInfo)
{
	mDiffuse = Vector4::Normalize(materialInfo.mAlbedo);
	mSpecular = materialInfo.mSpecular; //(r,g,b,a=power)
	mSpecular.w = (materialInfo.mGlossiness * 255.0f);
	mEmissive = materialInfo.mEmissive;
}

Material::Material()
{
}

Material::~Material()
{
}

void Material::UpdateShaderVariable()
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

void Material::LoadTextureFromFile(UINT nType, FILE* file)
{
	std::string textureName{};
	::ReadStringFromFile(file, textureName);
	
	if (textureName == "null") {
		return;
	}

	mTexture = crntScene->GetTexture(textureName);
}



// [ MasterModel ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MasterModel::MasterModel()
{
	mMesh = std::make_shared<MergedMesh>();
}


MasterModel::~MasterModel()
{
	
}

void MasterModel::ReleaseUploadBuffers()
{
	mMesh->ReleaseUploadBuffers();
}

Vec4 MasterModel::GetColor() const
{
	return Vec4(1.0f, 1.0f, 0.0f, 1.0f);
}

rsptr<Texture> MasterModel::GetTexture() const
{
	return mMesh->GetTexture();
}

void MasterModel::SetModel(rsptr<Model> model)
{
	SetChild(model);
}

void MasterModel::MergeMesh(rsptr<MeshLoadInfo> mesh, const std::vector<sptr<Material>>& materials)
{
	mMesh->MergeMesh(mesh, materials);
}


void MasterModel::Close()
{
	mMesh->Close();
}

void MasterModel::Render(const GameObject* gameObject) const
{
	RenderFunc(gameObject);
}

void MasterModel::Render(const ObjectInstanceBuffer* instBuffer) const
{
	mMesh->Render(instBuffer);
}

void MasterModel::RenderObject(const GameObject* gameObject) const
{
	mMesh->Render(gameObject);
}

void MasterModel::RenderSprite(const GameObject* gameObject) const
{
	mMesh->RenderSprite(gameObject);
}

void MasterModel::CopyModelHierarchy(GameObject* object) const
{
	GetModel()->CopyModelHierarchy(object);
}