#include "stdafx.h"
#include "FileMgr.h"
#include "DXGIMgr.h"

#include "Model.h"
#include "Mesh.h"
#include "Light.h"
#include "Collider.h"

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40









sptr<CMeshLoadInfo> FileMgr::LoadMeshInfoFromFile(FILE* file)
{
	std::string token;
	UINT nReads = 0;

	int nPositions = 0, nColors = 0, nNormals = 0, nIndices = 0, nSubMeshes = 0, nSubIndices = 0;

	sptr<CMeshLoadInfo> meshInfo = std::make_shared<CMeshLoadInfo>();

	meshInfo->mVertexCount = ::ReadIntegerFromFile(file);
	::ReadStringFromFile(file, meshInfo->mMeshName);

	while (true) {
		::ReadStringFromFile(file, token);

		if (token == "<Positions>:") {
			nPositions = ::ReadIntegerFromFile(file);
			if (nPositions > 0) {
				meshInfo->mType |= static_cast<DWORD>(VertexType::Position);
				meshInfo->mVertices.resize(nPositions);
				nReads = (UINT)::fread(meshInfo->mVertices.data(), sizeof(XMFLOAT3), nPositions, file);
			}
		}
		if (token == "<Colors>:") {
			nColors = ::ReadIntegerFromFile(file);
			if (nColors > 0) {
				std::vector<Vec4> colors{};
				colors.resize(nColors);
				nReads = (UINT)::fread(colors.data(), sizeof(XMFLOAT4), nColors, file);
			}
		}
		else if (token == "<Normals>:") {
			nNormals = ::ReadIntegerFromFile(file);
			if (nNormals > 0) {
				meshInfo->mType |= static_cast<DWORD>(VertexType::Normal);
				meshInfo->mNormals.resize(nNormals);
				nReads = (UINT)::fread(meshInfo->mNormals.data(), sizeof(XMFLOAT3), nNormals, file);
			}
		}
		else if (token == "<Indices>:") {
			nIndices = ::ReadIntegerFromFile(file);
			if (nIndices > 0) {
				meshInfo->mIndices.resize(nIndices);
				nReads = (UINT)::fread(meshInfo->mIndices.data(), sizeof(int), nIndices, file);
			}
		}
		else if (token == "<SubMeshes>:") {
			meshInfo->mSubMeshCount = ::ReadIntegerFromFile(file);
			if (meshInfo->mSubMeshCount > 0) {
				meshInfo->mSubSetIndexCounts.resize(meshInfo->mSubMeshCount);
				meshInfo->mSubSetIndices.resize(meshInfo->mSubMeshCount);

				for (int i = 0; i < meshInfo->mSubMeshCount; i++) {
					::ReadStringFromFile(file, token);
					if (token == "<SubMesh>:") {
						int nIndex = ::ReadIntegerFromFile(file);
						meshInfo->mSubSetIndexCounts[i] = ::ReadIntegerFromFile(file);
						if (meshInfo->mSubSetIndexCounts[i] > 0) {
							meshInfo->mSubSetIndices[i].resize(meshInfo->mSubSetIndexCounts[i]);
							nReads = (UINT)::fread(meshInfo->mSubSetIndices[i].data(), sizeof(int), meshInfo->mSubSetIndexCounts[i], file);
						}
					}
				}
			}
		}
		else if (token == "<TextureCoords0>:")
		{
			int UVSize{};
			nReads = (UINT)::fread(&UVSize, sizeof(int), 1, file);
			if (UVSize > 0)
			{
				meshInfo->mType |= static_cast<DWORD>(VertexType::UV0);
				meshInfo->mUV0.resize(UVSize);
				nReads = (UINT)::fread(meshInfo->mUV0.data(), sizeof(XMFLOAT2), UVSize, file);
			}
		}
		else if (token == "<TextureCoords1>:")
		{
			int UVSize{};
			nReads = (UINT)::fread(&UVSize, sizeof(int), 1, file);
			if (UVSize > 0)
			{
				meshInfo->mType |= static_cast<DWORD>(VertexType::UV1);
				meshInfo->mUV1.resize(UVSize);
				nReads = (UINT)::fread(meshInfo->mUV1.data(), sizeof(XMFLOAT2), UVSize, file);
			}
		}
		else if (token == "<Tangents>:")
		{
			int nTangents{};
			nReads = (UINT)::fread(&nTangents, sizeof(int), 1, file);
			if (nTangents > 0) {
				meshInfo->mType |= static_cast<DWORD>(VertexType::Tangent);
				meshInfo->mTangents.resize(nTangents);
				nReads = (UINT)::fread(meshInfo->mTangents.data(), sizeof(XMFLOAT3), nTangents, file);
			}
		}
		else if (token == "<BiTangents>:")
		{
			int nBiTangents{};
			nReads = (UINT)::fread(&nBiTangents, sizeof(int), 1, file);
			if (nBiTangents > 0) {
				meshInfo->mType |= static_cast<DWORD>(VertexType::BiTangent);
				meshInfo->mBiTangents.resize(nBiTangents);
				nReads = (UINT)::fread(meshInfo->mBiTangents.data(), sizeof(XMFLOAT3), nBiTangents, file);
			}
		}
		else if (token == "</Mesh>") {
			break;
		}
	}
	return meshInfo;
}

std::vector<sptr<CMaterial>> FileMgr::LoadMaterialsFromFile(FILE* file)
{
	std::string token;
	UINT nReads = 0;

	int matIndex = 0;

	std::vector<sptr<CMaterial>> materials;

	int size = ::ReadIntegerFromFile(file);
	materials.resize(size);

	sptr<MATERIALLOADINFO> matInfo{};
	sptr<CMaterial> material{};

	while (true) {
		::ReadStringFromFile(file, token);

		if (token == "<Material>:") {
			if (matInfo) {
				sptr<CMaterialColors> materialColors = std::make_shared<CMaterialColors>(*matInfo);
				material->SetMaterialColors(materialColors);
			}
			matIndex = ::ReadIntegerFromFile(file);
			matInfo = std::make_shared<MATERIALLOADINFO>();
			materials[matIndex] = std::make_shared<CMaterial>();
			material = materials[matIndex];
		}
		else if (token == "<AlbedoColor>:") {
			nReads = (UINT)::fread(&(matInfo->mAlbedo), sizeof(float), 4, file);
		}
		else if (token == "<EmissiveColor>:") {
			nReads = (UINT)::fread(&(matInfo->mEmissive), sizeof(float), 4, file);
		}
		else if (token == "<SpecularColor>:") {
			nReads = (UINT)::fread(&(matInfo->mSpecular), sizeof(float), 4, file);
		}
		else if (token == "<Glossiness>:") {
			nReads = (UINT)::fread(&(matInfo->mGlossiness), sizeof(float), 1, file);
		}
		else if (token == "<Smoothness>:") {
			nReads = (UINT)::fread(&(matInfo->mSmoothness), sizeof(float), 1, file);
		}
		else if (token == "<Metallic>:") {
			nReads = (UINT)::fread(&(matInfo->mMetallic), sizeof(float), 1, file);
		}
		else if (token == "<SpecularHighlight>:") {
			nReads = (UINT)::fread(&(matInfo->mSpecularHighlight), sizeof(float), 1, file);
		}
		else if (token == "<GlossyReflection>:") {
			nReads = (UINT)::fread(&(matInfo->mGlossyReflection), sizeof(float), 1, file);
		}
		else if (token == "<AlbedoMap>:")
		{
			material->LoadTextureFromFile(MATERIAL_ALBEDO_MAP, file);
		}
		/*else if (token == "<SpecularMap>:")
		{
			material->LoadTextureFromFile(MATERIAL_SPECULAR_MAP, 4, pMaterial->m_ppstrTextureNames[1], &(pMaterial->m_ppTextures[1]), pParent, file);
		}
		else if (token == "<NormalMap>:")
		{
			material->LoadTextureFromFile(MATERIAL_NORMAL_MAP, 5, pMaterial->m_ppstrTextureNames[2], &(pMaterial->m_ppTextures[2]), pParent, file);
		}
		else if (token == "<MetallicMap>:")
		{
			material->LoadTextureFromFile(MATERIAL_METALLIC_MAP, 6, pMaterial->m_ppstrTextureNames[3], &(pMaterial->m_ppTextures[3]), pParent, file);
		}
		else if (token == "<EmissionMap>:")
		{
			material->LoadTextureFromFile(MATERIAL_EMISSION_MAP, 7, pMaterial->m_ppstrTextureNames[4], &(pMaterial->m_ppTextures[4]), pParent, file);
		}
		else if (token == "<DetailAlbedoMap>:")
		{
			material->LoadTextureFromFile(MATERIAL_DETAIL_ALBEDO_MAP, 8, pMaterial->m_ppstrTextureNames[5], &(pMaterial->m_ppTextures[5]), pParent, file);
		}
		else if (token == "<DetailNormalMap>:")
		{
			material->LoadTextureFromFile(MATERIAL_DETAIL_NORMAL_MAP, 9, pMaterial->m_ppstrTextureNames[6], &(pMaterial->m_ppTextures[6]), pParent, file);
		}*/
		else if (token == "</Materials>") {
			if (matInfo) {
				sptr<CMaterialColors> materialColors = std::make_shared<CMaterialColors>(*matInfo);
				material->SetMaterialColors(materialColors);
			}
			break;
		}
	}
	return materials;
}

sptr<CModel> FileMgr::LoadFrameHierarchyFromFile(FILE* file)
{
	std::string token;
	UINT nReads = 0;

	int nFrame = 0;
	int nTextures = 0;

	sptr<CModel> model{};

	while (true) {
		::ReadStringFromFile(file, token);

		if (token == "<Frame>:") {
			model = std::make_shared<CModel>();

			nFrame = ::ReadIntegerFromFile(file);
			nTextures = ::ReadIntegerFromFile(file);
			std::string name{};
			::ReadStringFromFile(file, name);
			model->SetName(name);
		}
		else if (token == "<Transform>:") {
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, file);
			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, file); //Euler Angle
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, file);
			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, file); //Quaternion
		}
		else if (token == "<TransformMatrix>:") {
			Vec4x4 transfrom;
			nReads = (UINT)::fread(&transfrom, sizeof(float), 16, file);
			model->SetTransform(transfrom);
		}
		else if (token == "<BoundingSphere>:") {
			MyBoundingSphere bs;
			nReads = (UINT)::fread(&bs.Center, sizeof(XMFLOAT3), 1, file);
			nReads = (UINT)::fread(&bs.Radius, sizeof(float), 1, file);
			bs.OriginCenter = bs.Center;
			model->AddComponent<SphereCollider>()->mBS = bs;
		}
		else if (token == "<BoundingBoxes>:") {
			int obbSize{};
			nReads = (UINT)::fread(&obbSize, sizeof(int), 1, file);

			std::vector<Vec3> centers(obbSize);
			std::vector<Vec3> extents(obbSize);
			nReads = (UINT)::fread(centers.data(), sizeof(float), 3 * obbSize, file);
			nReads = (UINT)::fread(extents.data(), sizeof(float), 3 * obbSize, file);

			MyBoundingOrientedBox box{};
			for (int i = 0; i < obbSize; ++i) {
				box.Center = centers[i];
				box.OriginCenter = centers[i];
				box.Extents = extents[i];
				model->AddComponent<BoxCollider>()->mBox = box;
			}
		}
		else if (token == "<Mesh>:") {
			model->SetMeshInfo(FileMgr::LoadMeshInfoFromFile(file));
		}
		else if (token == "<Materials>:") {
			model->SetMaterials(FileMgr::LoadMaterialsFromFile(file));
		}
		else if (token == "<Children>:") {
			int nChilds = ::ReadIntegerFromFile(file);
			if (nChilds > 0) {
				for (int i = 0; i < nChilds; i++) {
					sptr<CModel> child = FileMgr::LoadFrameHierarchyFromFile(file);
					if (child) {
						model->SetChild(child);
					}
				}
			}
		}
		else if (token == "</Frame>") {
			break;
		}
	}

	return model;
}

sptr<CMasterModel> FileMgr::LoadGeometryFromFile(const std::string& fileName)
{
	FILE* file = NULL;
	::fopen_s(&file, fileName.c_str(), "rb");
	::rewind(file);

	sptr<CModel> model = std::make_shared<CModel>();

	std::string token;

	while (true) {
		::ReadStringFromFile(file, token);

		if (token == "<Hierarchy>:") {
			model = FileMgr::LoadFrameHierarchyFromFile(file);
		}
		else if (token == "</Hierarchy>") {
			break;
		}
	}

	sptr<CMasterModel> masterModel = std::make_shared<CMasterModel>();
	model->MergeModel(*masterModel.get());
	masterModel->SetModel(model);
	masterModel->Close();
	return masterModel;
}






void FileMgr::LoadLightFromFile(const std::string& fileName, LIGHT** out)
{
	LIGHT* light = *out;

	FILE* file = NULL;
	::fopen_s(&file, fileName.c_str(), "rb");
	assert(file);
	::rewind(file);

	std::string token{};
	std::string name{};

	UINT nReads = 0;
	light->mAmbient = Vec4(0.0f, 0.0f, 0.0f, 1.0f);

	::ReadUnityBinaryString(file, token); // <Type:>
	::fread(&light->mType, sizeof(int), 1, file);

	::ReadUnityBinaryString(file, token); // <Color:>
	::fread(&light->mDiffuse, sizeof(Vec4), 1, file);

	float intensity;
	::ReadUnityBinaryString(file, token); // <Intensity:>
	//::fread(&light->mFalloff, sizeof(float), 1, file);
	::fread(&intensity, sizeof(float), 1, file);

	::ReadUnityBinaryString(file, token); // <Range:>
	::fread(&light->mRange, sizeof(float), 1, file);

	float angle{};
	::ReadUnityBinaryString(file, token); // <Angle:>
	::fread(&angle, sizeof(float), 1, file);

	angle /= 2.0f; // 보정값
	light->mPhi = (float)cos(XMConvertToRadians(angle));
	light->mTheta = (float)cos(XMConvertToRadians(angle / 2.0f));

	// 감쇠량
	Vec3 factor{};

	// intensity에 따른 감쇠도 설정
	if (intensity <= 3.0f) {
		factor.y = 0.01f * intensity;
	}
	else {
		factor.y = 0.05f / intensity;
	}
	factor.y *= 0.5f; // 보정값

	factor.z = factor.y * 0.01f;
	factor.x = factor.y * 100.0f;
	factor.x = min(1.0f, factor.x);

	light->mAttenuation = XMFLOAT3(factor.x, factor.y, factor.z); // 감쇠
	light->mFalloff = 1.0f;
	light->mSpecular = Vec4(0.1f, 0.1f, 0.1f, 1.0f);
}