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





//////////////////* I/O *//////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace IO {
	int ReadUnityBinaryString(FILE* file, std::string& token)
	{
		BYTE length{};
		token.resize(256);

		UINT nReads = 0;
		nReads = (UINT)::fread(&length, sizeof(BYTE), 1, file);
		nReads = (UINT)::fread(token.data(), sizeof(char), length, file);
		token.resize(length);

		return nReads;
	}

	int ReadIntegerFromFile(FILE* file)
	{
		int nValue{};
		UINT nReads = (UINT)::fread(&nValue, sizeof(int), 1, file);
		return nValue;
	}

	float ReadFloatFromFile(FILE* file)
	{
		float fValue{};
		UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, file);
		return fValue;
	}

	UINT ReadStringFromFile(FILE* file, std::string& token)
	{
		BYTE length{};
		UINT nReads{};

		token.resize(256);
		nReads = (UINT)::fread(&length, sizeof(BYTE), 1, file);
		nReads = (UINT)::fread(token.data(), sizeof(char), length, file);
		token.resize(length);

		if (nReads == 0) {
			throw std::runtime_error("Failed read file!\n");
		}

		return nReads;
	}

}



namespace FileMgr {
	sptr<MeshLoadInfo> LoadMeshInfoFromFile(FILE* file)
	{
		std::string token;
		UINT nReads = 0;

		int nPositions = 0, nColors = 0, nNormals = 0, nIndices = 0, nSubMeshes = 0, nSubIndices = 0;

		sptr<MeshLoadInfo> meshInfo = std::make_shared<MeshLoadInfo>();

		meshInfo->mVertexCount = IO::ReadIntegerFromFile(file);
		IO::ReadStringFromFile(file, meshInfo->mMeshName);

		while (true) {
			IO::ReadStringFromFile(file, token);

			switch (Hash(token)) {
			case Hash("<Positions>:"):
			{
				nPositions = IO::ReadIntegerFromFile(file);
				if (nPositions > 0) {
					meshInfo->mType |= static_cast<DWORD>(VertexType::Position);
					meshInfo->mVertices.resize(nPositions);
					nReads = (UINT)::fread(meshInfo->mVertices.data(), sizeof(Vec3), nPositions, file);
				}
			}
			break;
			case Hash("<Colors>:"):
			{
				nColors = IO::ReadIntegerFromFile(file);
				if (nColors > 0) {
					std::vector<Vec4> colors{};
					colors.resize(nColors);
					nReads = (UINT)::fread(colors.data(), sizeof(Vec4), nColors, file);
				}
			}
			break;
			case Hash("<Normals>:"):
			{
				nNormals = IO::ReadIntegerFromFile(file);
				if (nNormals > 0) {
					meshInfo->mType |= static_cast<DWORD>(VertexType::Normal);
					meshInfo->mNormals.resize(nNormals);
					nReads = (UINT)::fread(meshInfo->mNormals.data(), sizeof(Vec3), nNormals, file);
				}
			}
			break;
			case Hash("<Indices>:"):
			{
				nIndices = IO::ReadIntegerFromFile(file);
				if (nIndices > 0) {
					meshInfo->mIndices.resize(nIndices);
					nReads = (UINT)::fread(meshInfo->mIndices.data(), sizeof(int), nIndices, file);
				}
			}
			break;
			case Hash("<SubMeshes>:"):
			{
				meshInfo->mSubMeshCount = IO::ReadIntegerFromFile(file);
				if (meshInfo->mSubMeshCount > 0) {
					meshInfo->mSubSetIndexCounts.resize(meshInfo->mSubMeshCount);
					meshInfo->mSubSetIndices.resize(meshInfo->mSubMeshCount);

					for (int i = 0; i < meshInfo->mSubMeshCount; i++) {
						IO::ReadStringFromFile(file, token);
						if (token == "<SubMesh>:") {
							int nIndex = IO::ReadIntegerFromFile(file);
							meshInfo->mSubSetIndexCounts[i] = IO::ReadIntegerFromFile(file);
							if (meshInfo->mSubSetIndexCounts[i] > 0) {
								meshInfo->mSubSetIndices[i].resize(meshInfo->mSubSetIndexCounts[i]);
								nReads = (UINT)::fread(meshInfo->mSubSetIndices[i].data(), sizeof(int), meshInfo->mSubSetIndexCounts[i], file);
							}
						}
					}
				}
			}
			break;
			case Hash("<TextureCoords0>:"):
			{
				int uvSize{};
				nReads = (UINT)::fread(&uvSize, sizeof(int), 1, file);
				if (uvSize > 0)
				{
					meshInfo->mType |= static_cast<DWORD>(VertexType::UV0);
					meshInfo->mUV0.resize(uvSize);
					nReads = (UINT)::fread(meshInfo->mUV0.data(), sizeof(Vec2), uvSize, file);
				}
			}
			break;
			case Hash("<TextureCoords1>:"):
			{
				int uvSize{};
				nReads = (UINT)::fread(&uvSize, sizeof(int), 1, file);
				if (uvSize > 0)
				{
					meshInfo->mType |= static_cast<DWORD>(VertexType::UV1);
					meshInfo->mUV1.resize(uvSize);
					nReads = (UINT)::fread(meshInfo->mUV1.data(), sizeof(Vec2), uvSize, file);
				}
			}
			break;
			case Hash("<Tangents>:"):
			{
				int nTangents{};
				nReads = (UINT)::fread(&nTangents, sizeof(int), 1, file);
				if (nTangents > 0) {
					meshInfo->mType |= static_cast<DWORD>(VertexType::Tangent);
					meshInfo->mTangents.resize(nTangents);
					nReads = (UINT)::fread(meshInfo->mTangents.data(), sizeof(Vec3), nTangents, file);
				}
			}
			break;
			case Hash("<BiTangents>:"):
			{
				int nBiTangents{};
				nReads = (UINT)::fread(&nBiTangents, sizeof(int), 1, file);
				if (nBiTangents > 0) {
					meshInfo->mType |= static_cast<DWORD>(VertexType::BiTangent);
					meshInfo->mBiTangents.resize(nBiTangents);
					nReads = (UINT)::fread(meshInfo->mBiTangents.data(), sizeof(Vec3), nBiTangents, file);
				}
			}
			break;
			case Hash("</Mesh>"):
				goto end_loop;
				break;

			default:
				assert(0);
				break;
			}
		}
	end_loop:

		return meshInfo;
	}

	std::vector<sptr<Material>> LoadMaterialsFromFile(FILE* file)
	{
		std::string token;
		UINT nReads = 0;

		int matIndex = 0;

		std::vector<sptr<Material>> materials;

		int size = IO::ReadIntegerFromFile(file);
		materials.resize(size);

		sptr<MATERIALLOADINFO> matInfo{};
		sptr<Material> material{};

		while (true) {
			IO::ReadStringFromFile(file, token);

			switch (Hash(token)) {
			case Hash("<Material>:"):
			{
				if (matInfo) {
					sptr<MaterialColors> materialColors = std::make_shared<MaterialColors>(*matInfo);
					material->SetMaterialColors(materialColors);
				}
				matIndex = IO::ReadIntegerFromFile(file);
				matInfo = std::make_shared<MATERIALLOADINFO>();
				materials[matIndex] = std::make_shared<Material>();
				material = materials[matIndex];
			}
			break;
			case Hash("<AlbedoColor>:"):
				nReads = (UINT)::fread(&(matInfo->mAlbedo), sizeof(float), 4, file);
				break;
			case Hash("<EmissiveColor>:"):
				nReads = (UINT)::fread(&(matInfo->mEmissive), sizeof(float), 4, file);
				break;
			case Hash("<SpecularColor>:"):
				nReads = (UINT)::fread(&(matInfo->mSpecular), sizeof(float), 4, file);
				break;
			case Hash("<Glossiness>:"):
				nReads = (UINT)::fread(&(matInfo->mGlossiness), sizeof(float), 1, file);
				break;
			case Hash("<Smoothness>:"):
				nReads = (UINT)::fread(&(matInfo->mSmoothness), sizeof(float), 1, file);
				break;
			case Hash("<Metallic>:"):
				nReads = (UINT)::fread(&(matInfo->mMetallic), sizeof(float), 1, file);
				break;
			case Hash("<SpecularHighlight>:"):
				nReads = (UINT)::fread(&(matInfo->mSpecularHighlight), sizeof(float), 1, file);
				break;
			case Hash("<GlossyReflection>:"):
				nReads = (UINT)::fread(&(matInfo->mGlossyReflection), sizeof(float), 1, file);
				break;
			case Hash("<AlbedoMap>:"):
				material->LoadTextureFromFile(MATERIAL_ALBEDO_MAP, file);
				break;
			case Hash("</Materials>"):
			{
				if (matInfo) {
					sptr<MaterialColors> materialColors = std::make_shared<MaterialColors>(*matInfo);
					material->SetMaterialColors(materialColors);
				}
			}
			goto end_loop;
			break;

			default:	// 다른 Map들은 import하지 않을 수 있다.
				break;
			}
		}
	end_loop:

		return materials;
	}

	sptr<Model> LoadFrameHierarchyFromFile(FILE* file)
	{
		std::string token;
		UINT nReads = 0;

		int nFrame = 0;
		int nTextures = 0;

		sptr<Model> model{};

		while (true) {
			IO::ReadStringFromFile(file, token);

			switch (Hash(token)) {
			case Hash("<Frame>:"):
			{
				model = std::make_shared<Model>();

				nFrame = IO::ReadIntegerFromFile(file);
				nTextures = IO::ReadIntegerFromFile(file);
				std::string name{};
				IO::ReadStringFromFile(file, name);
				model->SetName(name);
			}
			break;
			case Hash("<Transform>:"):
			{
				Vec3 xmf3Position, xmf3Rotation, xmf3Scale;
				Vec4 xmf4Rotation;
				nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, file);
				nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, file); //Euler Angle
				nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, file);
				nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, file); //Quaternion
			}
			break;
			case Hash("<TransformMatrix>:"):
			{
				Vec4x4 transfrom;
				nReads = (UINT)::fread(&transfrom, sizeof(float), 16, file);
				model->SetTransform(transfrom);
			}
			break;
			case Hash("<BoundingSphere>:"):
			{
				MyBoundingSphere bs;
				nReads = (UINT)::fread(&bs.Center, sizeof(Vec3), 1, file);
				nReads = (UINT)::fread(&bs.Radius, sizeof(float), 1, file);
				bs.OriginCenter = bs.Center;
				model->AddComponent<SphereCollider>()->mBS = bs;
			}
			break;
			case Hash("<BoundingBoxes>:"):
			{
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
			break;
			case Hash("<Mesh>:"):
			{
				model->SetMeshInfo(FileMgr::LoadMeshInfoFromFile(file));
			}
			break;
			case Hash("<Materials>:"):
			{
				model->SetMaterials(FileMgr::LoadMaterialsFromFile(file));
			}
			break;
			case Hash("<Children>:"):
			{
				int nChilds = IO::ReadIntegerFromFile(file);
				if (nChilds > 0) {
					for (int i = 0; i < nChilds; i++) {
						sptr<Model> child = FileMgr::LoadFrameHierarchyFromFile(file);
						if (child) {
							model->SetChild(child);
						}
					}
				}
			}
			break;
			case Hash("</Frame>"):
				goto end_loop;
				break;

			default:
				//assert(0);
				break;
			}
		}
	end_loop:

		return model;
	}

	sptr<MasterModel> LoadGeometryFromFile(const std::string& fileName)
	{
		FILE* file = nullptr;
		::fopen_s(&file, fileName.c_str(), "rb");
		::rewind(file);

		sptr<Model> model = std::make_shared<Model>();

		std::string token;

		while (true) {
			IO::ReadStringFromFile(file, token);

			switch (Hash(token)) {
			case Hash("<Hierarchy>:"):
				model = FileMgr::LoadFrameHierarchyFromFile(file);
				break;
			case Hash("</Hierarchy>"):
				goto end_loop;
				break;
			default:
				assert(0);
				break;
			}
		}
	end_loop:

		sptr<MasterModel> masterModel = std::make_shared<MasterModel>();
		model->MergeModel(*masterModel.get());
		masterModel->SetModel(model);
		masterModel->Close();

		return masterModel;
	}



	void LoadLightFromFile(const std::string& fileName, LIGHT** out)
	{
		LIGHT* light = *out;

		FILE* file = nullptr;
		::fopen_s(&file, fileName.c_str(), "rb");
		assert(file);
		::rewind(file);

		std::string token{};
		std::string name{};

		UINT nReads = 0;
		light->mAmbient = Vec4(0.0f, 0.0f, 0.0f, 1.0f);

		IO::ReadUnityBinaryString(file, token); // <Type:>
		::fread(&light->mType, sizeof(int), 1, file);

		IO::ReadUnityBinaryString(file, token); // <Color:>
		::fread(&light->mDiffuse, sizeof(Vec4), 1, file);

		float intensity;
		IO::ReadUnityBinaryString(file, token); // <Intensity:>
		::fread(&intensity, sizeof(float), 1, file);

		IO::ReadUnityBinaryString(file, token); // <Range:>
		::fread(&light->mRange, sizeof(float), 1, file);

		float angle{};
		IO::ReadUnityBinaryString(file, token); // <Angle:>
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

		light->mAttenuation = Vec3(factor.x, factor.y, factor.z); // 감쇠
		light->mFalloff = 1.0f;
		light->mSpecular = Vec4(0.1f, 0.1f, 0.1f, 1.0f);
	}


	void GetTextureNames(std::vector<std::string>& out, const std::string& folder)
	{
		for (const auto& file : std::filesystem::directory_iterator(folder)) {
			std::string fileName = file.path().filename().string();
			fileName.erase(fileName.size() - 4); // remove .dds
			out.emplace_back(fileName);
		}
	}
}