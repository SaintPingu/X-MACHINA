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
#include "FileIO.h"




#pragma region MaterialColors
MaterialColors::MaterialColors(const MaterialLoadInfo& materialInfo)
{
	mDiffuse    = Vector4::Normalize(materialInfo.Albedo);
	mSpecular   = materialInfo.Specular; //(r,g,b,a=power)
	mSpecular.w = materialInfo.Glossiness * 255.0f;
	mEmissive   = materialInfo.Emissive;
}
#pragma endregion


#pragma region Material
void Material::UpdateShaderVars()
{
	constexpr RootParam param = RootParam::GameObjectInfo;

	if (mTexture) {
		mTexture->UpdateShaderVars();
	}
	else {
		scene->SetGraphicsRoot32BitConstants(param, (WORD)MaterialMap::None, 32);
	}

	if (!mMaterialColors) {
		Vec4 ambient{}, diffuse{}, specular{}, emmisive{};

		scene->SetGraphicsRoot32BitConstants(param, ambient, 16);
		scene->SetGraphicsRoot32BitConstants(param, diffuse, 20);
		scene->SetGraphicsRoot32BitConstants(param, specular, 24);
		scene->SetGraphicsRoot32BitConstants(param, emmisive, 28);

		return;
	}

	scene->SetGraphicsRoot32BitConstants(param, mMaterialColors->mDiffuse, 20);

	if (mIsDiffused) {
		return;
	}

	scene->SetGraphicsRoot32BitConstants(param, mMaterialColors->mAmbient, 16);
	scene->SetGraphicsRoot32BitConstants(param, mMaterialColors->mSpecular, 24);
	scene->SetGraphicsRoot32BitConstants(param, mMaterialColors->mEmissive, 28);
}

void Material::LoadTextureFromFile(FILE* file)
{
	std::string textureName{};
	FileIO::ReadString(file, textureName);

	if (textureName == "null") {
		return;
	}

	mTexture = scene->GetTexture(textureName);
}
#pragma endregion


#pragma region Model
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
#pragma endregion


#pragma region MasterModel
#pragma endregion










// [ MasterModel ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MasterModel::MasterModel()
{
	mMesh = std::make_shared<MergedMesh>();
}

rsptr<Texture> MasterModel::GetTexture() const
{
	return mMesh->GetTexture();
}

void MasterModel::SetModel(rsptr<Model> model)
{
	SetChild(model);
}


void MasterModel::ReleaseUploadBuffers()
{
	mMesh->ReleaseUploadBuffers();
}

void MasterModel::MergeMesh(rsptr<MeshLoadInfo> mesh, const std::vector<sptr<Material>>& materials)
{
	mMesh->MergeMesh(mesh, materials);
}

void MasterModel::Close()
{
	mMesh->Close();
}

void MasterModel::Render(const ObjectInstanceBuffer* instBuffer) const
{
	mMesh->Render(instBuffer);
}

void MasterModel::CopyModelHierarchy(GameObject* object) const
{
	GetModel()->CopyModelHierarchy(object);
}


void MasterModel::RenderObject(const GameObject* gameObject) const
{
	mMesh->Render(gameObject);
}

void MasterModel::RenderSprite(const GameObject* gameObject) const
{
	mMesh->RenderSprite(gameObject);
}