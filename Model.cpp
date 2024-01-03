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

	// texture�� �ִٸ�, texture�� shader variables�� ������Ʈ�ϰ�
	//			 ���ٸ�, textureMask�� None���� �����Ѵ�.
	if (mTexture) {
		mTexture->UpdateShaderVars();
	}
	else {
		scene->SetGraphicsRoot32BitConstants(kRootParam, static_cast<DWORD>(MaterialMap::None), 32);
	}

	// material ���� ���ٸ�, ����(0,0,0=black)���� �����Ѵ�.
	if (!mMaterialColors) {
		scene->SetGraphicsRoot32BitConstants(kRootParam, Vec4x4{}, 16);
		return;
	}

	// material�� ���� (Vec4x4 -> Ambient, Diffuse, Specular, Emissive)�� Set�Ѵ�.
	scene->SetGraphicsRoot32BitConstants(kRootParam, *mMaterialColors, 16);
}

// texture�� fileName�� �о�� �ش� texture�� scene�� ���� �����´�.
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
// ����Լ�
void Model::CopyModelHierarchy(Object* object) const
{
	object->CopyComponents(*this);
	object->SetLocalTransform(GetLocalTransform());
	object->SetName(mName);

	/* �� ���� ������ ���纻(�� �Ҵ�)�� �޾� �����Ѵ�. */
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
	out.MergeMesh(mMeshInfo, mMaterials);	// mesh�� material�� out�� �����Ѵ�.

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