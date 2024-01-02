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
	:
	Diffuse(Vector4::Normalize(materialInfo.Albedo)),
	Specular(materialInfo.Specular),
	Emissive(materialInfo.Emissive)
{
	Specular.w = materialInfo.Glossiness * 255.0f;
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

	scene->SetGraphicsRoot32BitConstants(param, mMaterialColors->Diffuse, 20);

	if (mIsDiffused) {
		return;
	}

	scene->SetGraphicsRoot32BitConstants(param, mMaterialColors->Ambient, 16);
	scene->SetGraphicsRoot32BitConstants(param, mMaterialColors->Specular, 24);
	scene->SetGraphicsRoot32BitConstants(param, mMaterialColors->Emissive, 28);
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
	object->SetLocalTransform(GetLocalTransform());
	object->SetName(mName);


	if (mSibling) {
		sptr<Object> sibling{};
		mSibling->GetObj<Model>()->CopyModelHierarchy(sibling);
		object->mSibling = sibling;
	}
	if (mChild) {
		sptr<Object> child{};
		mChild->GetObj<Model>()->CopyModelHierarchy(child);
		object->SetChild(child);
	}
}

void Model::MergeModel(MasterModel& out)
{
	out.MergeMesh(mMeshInfo, mMaterials);

	if (mSibling) {
		mSibling->GetObj<Model>()->MergeModel(out);
	}
	if (mChild) {
		mChild->GetObj<Model>()->MergeModel(out);
	}
}

void Model::CopyModelHierarchy(sptr<Object>& object) const
{
	object = std::make_shared<Object>();

	object->CopyComponents(*this);
	object->SetLocalTransform(GetLocalTransform());
	object->SetName(mName);

	if (mSibling) {
		sptr<Object> sibling{};
		mSibling->GetObj<Model>()->CopyModelHierarchy(sibling);
		object->mSibling = sibling;
	}
	if (mChild) {
		sptr<Object> child{};
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

void MasterModel::ReleaseUploadBuffers()
{
	mMesh->ReleaseUploadBuffers();
}

void MasterModel::SetModel(const rsptr<Model> model)
{
	mModel = model;
	model->MergeModel(*this);
	mMesh->StopMerge();

	mMerged = true;
}

void MasterModel::MergeMesh(sptr<MeshLoadInfo>& mesh, std::vector<sptr<Material>>& materials)
{
	assert(!mMerged);

	mMesh->MergeMesh(mesh, materials);
}

void MasterModel::Render(const ObjectInstBuffer* instBuffer) const
{
	mMesh->Render(instBuffer);
}

void MasterModel::CopyModelHierarchy(GameObject* object) const
{
	mModel->CopyModelHierarchy(object);
}


void MasterModel::RenderObject(const GameObject* object) const
{
	mMesh->Render(object);
}

void MasterModel::RenderSprite(const GameObject* object) const
{
	mMesh->RenderSprite(object);
}