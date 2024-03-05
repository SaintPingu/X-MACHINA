#include "stdafx.h"
#include "Model.h"
#include "DXGIMgr.h"
#include "FrameResource.h"

#include "Object.h"
#include "Mesh.h"
#include "Texture.h"
#include "Scene.h"
#include "FileIO.h"

#include "Animator.h"


#pragma region MaterialColors
MaterialColors::MaterialColors(const MaterialLoadInfo& materialInfo)
	:
	Diffuse(Vector4::NormalizeColor(materialInfo.Albedo)),
	Specular(materialInfo.Specular),
	Emissive(materialInfo.Emissive)
{
	Specular.w = materialInfo.Glossiness * 255.0f;
}
#pragma endregion


#pragma region Material
void Material::UpdateShaderVars()
{
	MaterialData materialData;
	materialData.Ambient			= mMaterialColors->Ambient;
	materialData.Diffuse			= mMaterialColors->Diffuse;
	materialData.Specular			= mMaterialColors->Specular;
	materialData.Emissive			= mMaterialColors->Emissive;

	for (UINT8 i = 0; i < TextureMapCount; ++i) {
		if (mTextures[i]) {
			materialData.MapIndices[i] = mTextures[i]->GetGpuDescriptorHandleIndex();
		}
	}

	frmResMgr->CopyData(mMatIndex, materialData);
}

void Material::LoadTextureFromFile(TextureMap map, FILE* file)
{
	std::string textureName{};
	FileIO::ReadString(file, textureName);

	if (textureName == "null") {
		return;
	}

	mTextures[static_cast<UINT8>(map)] = scene->GetTexture(textureName);
}
#pragma endregion


#pragma region Model
// 재귀함수
void Model::CopyModelHierarchy(Object* object, Object* parent) const
{
	object->CopyComponents(*this);
	object->SetLocalTransform(GetLocalTransform());
	object->SetName(GetName());
	object->mParent = parent;

	/* 각 계층 구조의 복사본(새 할당)을 받아 설정한다. */
	if (mSibling) {
		sptr<Object> sibling = std::make_shared<Object>();
		mSibling->GetObj<Model>()->CopyModelHierarchy(sibling.get(), parent);
		object->mSibling = sibling;
	}
	if (mChild) {
		sptr<Object> child = std::make_shared<Object>();
		mChild->GetObj<Model>()->CopyModelHierarchy(child.get(), object);
		object->SetChild(child);
	}
}

void Model::MergeModel(MasterModel& out)
{
	out.MergeMesh(mMeshInfo, mMaterials);	// mesh와 material을 out에 병합한다.

	if (mSibling) {
		mSibling->GetObj<Model>()->MergeModel(out);
	}
	if (mChild) {
		mChild->GetObj<Model>()->MergeModel(out);
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
	:
	mMesh(std::make_shared<MergedMesh>())
{
	
}

rsptr<Texture> MasterModel::GetTexture() const
{
	return mMesh->GetTexture();
}

void MasterModel::SetAnimationInfo(sptr<AnimationLoadInfo> animationInfo)
{
	mAnimationInfo = animationInfo;
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

void MasterModel::MergeMesh(sptr<MeshLoadInfo>& mesh, std::vector<sptr<Material>>& materials) const
{
	assert(!mMerged);

	mMesh->MergeMesh(mesh, materials);
}

void MasterModel::Render(const ObjectPool* objectPool) const
{
	mMesh->Render(objectPool);
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