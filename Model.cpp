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
	constexpr RootParam kRootParam = RootParam::GameObjectInfo;

	// texture가 있다면, texture의 shader variables를 업데이트하고
	//			 없다면, textureMask를 None으로 설정한다.
	if (mTexture) {
		mTexture->UpdateShaderVars();
	}
	else {
		scene->SetGraphicsRoot32BitConstants(kRootParam, static_cast<DWORD>(MaterialMap::None), 32);
	}

	// material 색이 없다면, 무색(0,0,0=black)으로 설정한다.
	if (!mMaterialColors) {
		scene->SetGraphicsRoot32BitConstants(kRootParam, Vec4x4{}, 16);
		return;
	}

	// material의 색상 (Vec4x4 -> Ambient, Diffuse, Specular, Emissive)을 Set한다.
	scene->SetGraphicsRoot32BitConstants(kRootParam, *mMaterialColors, 16);
}

// texture의 fileName을 읽어와 해당 texture를 scene을 통해 가져온다.
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
// 재귀함수
void Model::CopyModelHierarchy(Object* object) const
{
	object->CopyComponents(*this);
	object->SetLocalTransform(GetLocalTransform());
	object->SetName(mName);

	/* 각 계층 구조의 복사본(새 할당)을 받아 설정한다. */
	if (mSibling) {
		sptr<Object> sibling = std::make_shared<Object>();
		mSibling->GetObj<Model>()->CopyModelHierarchy(sibling.get());
		object->mSibling = sibling;
	}
	if (mChild) {
		sptr<Object> child = std::make_shared<Object>();
		mChild->GetObj<Model>()->CopyModelHierarchy(child.get());
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