#include "EnginePch.h"
#include "Model.h"
#include "DXGIMgr.h"
#include "FrameResource.h"

#include "Object.h"
#include "Mesh.h"
#include "Texture.h"
#include "FileIO.h"
#include "ResourceMgr.h"


#pragma region MaterialColors
MaterialColors::MaterialColors(const MaterialLoadInfo& materialInfo)
	:
	DiffuseAlbedo(materialInfo.DiffuseAlbedo),
	Emission(materialInfo.Emission),
	Metallic(materialInfo.Metallic),
	Roughness(materialInfo.Roughness),
	OcclusionMask(materialInfo.OcclusionMask),
	AlphaTest(materialInfo.AlphaTest)
{
}
#pragma endregion


#pragma region Material
void Material::UpdateShaderVars()
{
	MaterialData materialData;
	if (mMaterialColors) {
		materialData.DiffuseAlbedo = mMaterialColors->DiffuseAlbedo;
		materialData.Emission = mMaterialColors->Emission;
		materialData.Metallic = mMaterialColors->Metallic;
		materialData.Roughness = mMaterialColors->Roughness;
		materialData.OcclusionMask = mMaterialColors->OcclusionMask;
		materialData.AlphaTest = mMaterialColors->AlphaTest;
	}

	for (UINT8 i = 0; i < TextureMapCount; ++i) {
		if (mTextures[i]) {
			materialData.MapIndices[i] = mTextures[i]->GetSrvIdx();
		}
	}

	FRAME_RESOURCE_MGR->CopyData(mMatIndex, materialData);
}

void Material::LoadTextureFromFile(TextureMap map, std::ifstream& file)
{
	std::string textureName{};
	FileIO::ReadString(file, textureName);

	if (textureName == "null") {
		return;
	}

	mTextures[static_cast<UINT8>(map)] = RESOURCE<Texture>(textureName);
}
#pragma endregion


#pragma region Model
void Model::SetMeshInfo(rsptr<MeshLoadInfo> meshInfo)
{
	mMeshInfo = meshInfo;
	if(mMeshInfo)
	{
		mHasMesh = true;
		if (mMeshInfo->SkinMesh)
		{
			mIsSkinMesh = true;
		}
	}
}
// 재귀함수
void Model::CopyModelHierarchy(Object* object, Object* parent) const
{
	object->CopyComponents(*this);
	object->SetLocalTransform(GetLocalTransform());
	object->SetName(GetName());
	object->mParent = parent;

	if (mHasMesh) {
		object->SetHasMesh();
	}

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

void Model::MergeModel(MasterModel& out, bool& isSkinMesh)
{
	if (mHasMesh && !mIsSkinMesh) {
		isSkinMesh = false;
	}
	out.MergeMesh(mMeshInfo, mMaterials);	// mesh와 material을 out에 병합한다.

	if (mSibling) {
		mSibling->GetObj<Model>()->MergeModel(out, isSkinMesh);
	}
	if (mChild) {
		mChild->GetObj<Model>()->MergeModel(out, isSkinMesh);
	}
}
void Model::MergeTransform(std::vector<Transform*>& out, Model* transform)
{
	if (transform->mMeshInfo) {
		out.emplace_back(transform);
	}

	if (transform->mSibling) {
		Model::MergeTransform(out, transform->mSibling->GetObj<Model>());
	}
	if (transform->mChild) {
		Model::MergeTransform(out, transform->mChild->GetObj<Model>());
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
	Resource(ResourceType::Model),
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
	MergeModelTransforms();
	mIsSkinMesh = true;
	model->MergeModel(*this, mIsSkinMesh);
	mMesh->StopMerge();

	mMerged = true;
}

void MasterModel::MergeMesh(sptr<MeshLoadInfo>& mesh, std::vector<sptr<Material>>& materials) const
{
	assert(!mMerged);

	mMesh->MergeMesh(mesh, materials);
}

void MasterModel::Render(const GameObject* object) const
{
	mMesh->Render(object);
}

void MasterModel::Render(const ObjectPool* objectPool) const
{
	mMesh->Render(objectPool);
}

void MasterModel::CopyModelHierarchy(GameObject* object) const
{
	mModel->CopyModelHierarchy(object);
}

void MasterModel::MergeModelTransforms()
{
	Model::MergeTransform(mMergedTransform, mModel.get());
}