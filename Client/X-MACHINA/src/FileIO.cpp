#include "stdafx.h"
#include "FileIO.h"
#include "DXGIMgr.h"

#include "Scene.h"
#include "Model.h"
#include "Mesh.h"
#include "Light.h"
#include "Collider.h"
#include "Texture.h"
#include "Animator.h"
#include "AnimationClip.h"
#include "AnimatorState.h"
#include "AnimatorController.h"
#include "AnimatorLayer.h"


namespace {
	// Mesh 정보를 불러온다. (Positions, Normals, ...)
	sptr<MeshLoadInfo> LoadMesh(FILE* file)
	{
		std::string token;

		int nPositions = 0, nColors = 0, nNormals = 0, nIndices = 0;

		sptr<MeshLoadInfo> meshInfo = std::make_shared<MeshLoadInfo>();

		FileIO::ReadVal(file, meshInfo->VertexCnt);
		FileIO::ReadString(file, meshInfo->MeshName);

		bool isEOF{ false };
		while (!isEOF) {
			FileIO::ReadString(file, token);

			switch (Hash(token)) {
			case Hash("<Positions>:"):
			{
				FileIO::ReadVal(file, nPositions);
				if (nPositions > 0) {
					meshInfo->VertexType |= VertexType::Position;
					FileIO::ReadRange(file, meshInfo->Buffer.Vertices, nPositions);
				}
			}

			break;
			case Hash("<Colors>:"):
			{
				FileIO::ReadVal(file, nColors);
				if (nColors > 0) {
					meshInfo->VertexType |= VertexType::Color;
					std::vector<Vec4> colors{};
					FileIO::ReadRange(file, colors, nColors);
				}
			}

			break;
			case Hash("<Normals>:"):
			{
				FileIO::ReadVal(file, nNormals);
				if (nNormals > 0) {
					meshInfo->VertexType |= VertexType::Normal;
					FileIO::ReadRange(file, meshInfo->Buffer.Normals, nNormals);
				}
			}

			break;
			case Hash("<Indices>:"):
			{
				FileIO::ReadVal(file, nIndices);
				if (nIndices > 0) {
					FileIO::ReadRange(file, meshInfo->Buffer.Indices, nIndices);
				}
			}

			break;
			case Hash("<SubMeshes>:"):
			{
				FileIO::ReadVal(file, meshInfo->SubMeshCnt);
				if (meshInfo->SubMeshCnt > 0) {
					meshInfo->SubSetIndexCnts.resize(meshInfo->SubMeshCnt);
					meshInfo->SubSetIndices.resize(meshInfo->SubMeshCnt);

					for (int i = 0; i < meshInfo->SubMeshCnt; i++) {
						FileIO::ReadString(file, token);
						if (token == "<SubMesh>:") {
							int nIndex = FileIO::ReadVal<int>(file);
							FileIO::ReadVal(file, meshInfo->SubSetIndexCnts[i]);
							if (meshInfo->SubSetIndexCnts[i] > 0) {
								FileIO::ReadRange(file, meshInfo->SubSetIndices[i], meshInfo->SubSetIndexCnts[i]);
							}
						}
					}
				}
			}

			break;
			case Hash("<TextureCoords0>:"):
			{
				int nUV0 = FileIO::ReadVal<int>(file);
				if (nUV0 > 0)
				{
					meshInfo->VertexType |= VertexType::UV0;
					FileIO::ReadRange(file, meshInfo->Buffer.UVs0, nUV0);
				}
			}

			break;
			case Hash("<TextureCoords1>:"):
			{
				int nUV1 = FileIO::ReadVal<int>(file);
				if (nUV1 > 0)
				{
					meshInfo->VertexType |= VertexType::UV1;
					FileIO::ReadRange(file, meshInfo->Buffer.UVs1, nUV1);
				}
			}

			break;
			case Hash("<Tangents>:"):
			{
				int nTangents = FileIO::ReadVal<int>(file);
				if (nTangents > 0) {
					meshInfo->VertexType |= VertexType::Tangent;
					FileIO::ReadRange(file, meshInfo->Buffer.Tangents, nTangents);
				}
			}

			break;
			case Hash("<BiTangents>:"):
			{
				int nBiTangents = FileIO::ReadVal<int>(file);
				if (nBiTangents > 0) {
					meshInfo->VertexType |= VertexType::BiTangent;
					FileIO::ReadRange(file, meshInfo->Buffer.BiTangents, nBiTangents);
				}
			}

			break;
			case Hash("</Mesh>"):
				isEOF = true;

			break;
			default:
				assert(0);
				break;
			}
		}

		return meshInfo;
	}



	sptr<SkinMesh> LoadSkinMesh(FILE* file, sptr<MeshLoadInfo>& meshInfo)
	{
		sptr<SkinMesh> mesh = std::make_shared<SkinMesh>();
		std::string token;

		bool isEOF = false;
		while (!isEOF) {
			FileIO::ReadString(file, token);

			switch (Hash(token)) {
			case Hash("<BoneNames>:"):
			{
				int skinBoneCnt{};
				FileIO::ReadVal(file, skinBoneCnt);
				if (skinBoneCnt > 0) {
					mesh->mBoneNames.resize(skinBoneCnt);
					for (int i = 0; i < skinBoneCnt; i++)
					{
						FileIO::ReadString(file, mesh->mBoneNames[i]);
					}
				}
			}

			break;
			case Hash("<BoneOffsets>:"):
			{
				int skinBoneCnt{};
				FileIO::ReadVal(file, skinBoneCnt);
				if (skinBoneCnt > 0) {
					std::vector<Vec4x4> boneOffsets(skinBoneCnt);
					FileIO::ReadRange(file, boneOffsets, skinBoneCnt);

					mesh->SetBoneOffsets(boneOffsets);
				}
			}

			break;
			case Hash("<BoneIndices>:"):
			{
				int vertexCnt = FileIO::ReadVal<int>(file);
				if (vertexCnt > 0)
				{
					auto& boneIndices = meshInfo->Buffer.BoneIndices;
					boneIndices.resize(vertexCnt);

					FileIO::ReadRange(file, boneIndices, vertexCnt);
				}
			}

			break;
			case Hash("<BoneWeights>:"):
			{
				int vertexCnt = FileIO::ReadVal<int>(file);
				if (vertexCnt > 0)
				{
					auto& boneWeights = meshInfo->Buffer.BoneWeights;
					boneWeights.resize(vertexCnt);

					FileIO::ReadRange(file, boneWeights, vertexCnt);
				}
			}

			break;
			case Hash("</SkinMesh>"):
				isEOF = true;

				break;
			default:
				assert(0);
				break;
			}
		}

		return mesh;
	}

	// 재질의 정보를 불러온다. (Albedo, Emissive, ...)
	std::vector<sptr<Material>> LoadMaterial(FILE* file)
	{
		std::string token;

		int matIndex = 0;

		std::vector<sptr<Material>> result;

		int size = FileIO::ReadVal<int>(file);
		result.resize(size);

		sptr<MaterialLoadInfo> matInfo{};
		sptr<Material> material{};

		bool isEOF{ false };
		while (!isEOF) {
			FileIO::ReadString(file, token);

			switch (Hash(token)) {
			case Hash("<Material>:"):
			{
				if (matInfo) {
					sptr<MaterialColors> materialColors = std::make_shared<MaterialColors>(*matInfo);
					material->SetMaterialColors(materialColors);
				}
				matIndex = FileIO::ReadVal<int>(file);
				matInfo = std::make_shared<MaterialLoadInfo>();
				result[matIndex] = std::make_shared<Material>();
				material = result[matIndex];
			}
			break;
			case Hash("<AlbedoColor>:"):
				FileIO::ReadVal(file, matInfo->Albedo);
				break;

			case Hash("<EmissiveColor>:"):
				FileIO::ReadVal(file, matInfo->Emissive);
				break;

			case Hash("<SpecularColor>:"):
				FileIO::ReadVal(file, matInfo->Specular);
				break;

			case Hash("<Glossiness>:"):
				FileIO::ReadVal(file, matInfo->Glossiness);
				break;

			case Hash("<Smoothness>:"):
				FileIO::ReadVal(file, matInfo->Smoothness);
				break;

			case Hash("<Metallic>:"):
				FileIO::ReadVal(file, matInfo->Metallic);
				break;

			case Hash("<SpecularHighlight>:"):
				FileIO::ReadVal(file, matInfo->SpecularHighlight);
				break;

			case Hash("<GlossyReflection>:"):
				FileIO::ReadVal(file, matInfo->GlossyReflection);
				break;

				// 여러 텍스처를 로드할 수 있도록 변경
			case Hash("<AlbedoMap>:"):
				material->LoadTextureFromFile(TextureMap::DiffuseMap0, file);
				break;

			case Hash("<NormalMap>:"):
				material->LoadTextureFromFile(TextureMap::NormalMap, file);
				break;

			case Hash("<HeightMap>:"):
				//material->LoadTextureFromFile(TextureMap::HeightMap, file);
				break;

			case Hash("<RoughnessMap>:"):
				//material->LoadTextureFromFile(TextureMap::RoughnessMap, file);
				break;

			case Hash("</Materials>"):
			{
				if (matInfo) {
					sptr<MaterialColors> materialColors = std::make_shared<MaterialColors>(*matInfo);
					material->SetMaterialColors(materialColors);
				}
				isEOF = true;
			}

			break;
			default:	// 다른 Map들은 import하지 않을 수 있다.
				break;
			}
		}

		return result;
	}


	// 한 프레임의 정보를 불러온다. (FrameName, Transform, BoundingBox, ...)
	sptr<Model> LoadFrameHierarchy(FILE* file, sptr<AnimationLoadInfo>& animationInfo)
	{
		std::string token;

		int nFrame = 0;
		int nTextures = 0;

		sptr<Model> model{};
		sptr<MeshLoadInfo> meshInfo{};

		bool isEOF{ false };
		while (!isEOF) {
			FileIO::ReadString(file, token);

			switch (Hash(token)) {
			case Hash("<Frame>:"):
			{
				model = std::make_shared<Model>();

				FileIO::ReadVal(file, nFrame);
				FileIO::ReadVal(file, nTextures);

				std::string name;
				FileIO::ReadString(file, name);

				model->SetName(name);
			}

			break;
			case Hash("<Transform>:"):
			{
				Vec3 position, rotation, scale;
				Vec4 quaternion;
				FileIO::ReadVal(file, position);
				FileIO::ReadVal(file, rotation);
				FileIO::ReadVal(file, scale);
				FileIO::ReadVal(file, quaternion);
			}

			break;
			case Hash("<TransformMatrix>:"):
			{
				Vec4x4 transform;
				FileIO::ReadVal(file, transform);

				model->SetLocalTransform(transform);
			}

			break;
			case Hash("<BoundingSphere>:"):
			{
				MyBoundingSphere bs;
				FileIO::ReadVal(file, bs.Center);
				FileIO::ReadVal(file, bs.Radius);
				bs.SetOrigin(bs.Center);

				model->AddComponent<SphereCollider>()->mBS = bs;
			}

			break;
			case Hash("<BoundingBoxes>:"):
			{
				int obbSize = FileIO::ReadVal<int>(file);

				std::vector<Vec3> centers;
				std::vector<Vec3> extents;
				FileIO::ReadRange(file, centers, obbSize);
				FileIO::ReadRange(file, extents, obbSize);

				MyBoundingOrientedBox box{};
				for (int i = 0; i < obbSize; ++i) {
					box.SetOrigin(centers[i]);
					box.Center = centers[i];
					box.Extents = extents[i];

					model->AddComponent<BoxCollider>()->mBox = box;
				}
			}

			break;
			case Hash("<Mesh>:"):
			{
				meshInfo = ::LoadMesh(file);
			}

			break;
			case Hash("<SkinMesh>:"):
			{
				FileIO::ReadString(file, token);	// <Mesh>:
				meshInfo = ::LoadMesh(file);
				meshInfo->SkinMesh = ::LoadSkinMesh(file, meshInfo);
				animationInfo->SkinMeshes.push_back(meshInfo->SkinMesh);
			}

			break;
			case Hash("<Materials>:"):
			{
				model->SetMaterials(::LoadMaterial(file));
			}

			break;
			case Hash("<Children>:"):
			{
				int nChilds = FileIO::ReadVal<int>(file);
				if (nChilds > 0) {
					for (int i = 0; i < nChilds; i++) {
						sptr<Model> child = LoadFrameHierarchy(file, animationInfo);
						if (child) {
							model->SetChild(child);
						}
					}
				}
			}

			break;
			case Hash("</Frame>"):
				isEOF = true;
				break;

			default:
				//assert(0);
				break;
			}
		}

		model->SetMeshInfo(meshInfo);
		return model;
	}


	// [folder]에 있는 모든 텍스쳐의 이름을 불러온다.
	void GetTextureNames(std::vector<std::string>& out, const std::string& folder)
	{
		for (const auto& file : std::filesystem::directory_iterator(folder)) {
			out.emplace_back(file.path().filename().string());
		}
	}


	std::vector<sptr<const AnimatorTransition>> LoadTransitions(FILE* file)
	{
		std::vector<sptr<const AnimatorTransition>> transitions{};

		int transitionSize = FileIO::ReadVal<int>(file);
		transitions.reserve(transitionSize);

		for (int i = 0; i < transitionSize; ++i) {
			sptr<AnimatorTransition> transition = std::make_shared<AnimatorTransition>();

			FileIO::ReadString(file, transition->Destination);
			int conditionSize = FileIO::ReadVal<int>(file);
			transition->Conditions.resize(conditionSize);

			for (auto& condition : transition->Conditions) {
				FileIO::ReadString(file, condition.mode);
				FileIO::ReadString(file, condition.paramName);

				switch (Hash(condition.mode)) {
				case Hash("If"):
				case Hash("IfNot"):
				case Hash("Trigger"):
					break;
				default:
					FileIO::ReadVal(file, condition.threshold);
					break;
				}
			}

			transitions.push_back(transition);
		}

		return transitions;
	}

	sptr<AnimatorLayer> LoadAnimatorLayer(FILE* file)
	{
		std::string token;

		std::string layerName;
		FileIO::ReadString(file, layerName);

		// Entry
		FileIO::ReadString(file, token);	// <Entry>:
		std::vector<sptr<const AnimatorTransition>> entryTransitions = LoadTransitions(file);

		sptr<AnimatorLayer> layer = std::make_shared<AnimatorLayer>(layerName, entryTransitions);

		// States //
		FileIO::ReadString(file, token);	// <States>:
		int stateSize = FileIO::ReadVal<int>(file);

		for (int i = 0; i < stateSize; ++i) {
			// AnimationClip
			std::string clipFolder, clipName;
			FileIO::ReadString(file, clipFolder);
			FileIO::ReadString(file, clipName);
			sptr<const AnimationClip> clip = scene->GetAnimationClip(clipFolder, clipName);

			// Transitions
			std::vector<sptr<const AnimatorTransition>> transitions = LoadTransitions(file);

			layer->AddState(std::make_shared<AnimatorState>(layer, clip, transitions));
		}

		// Sub Layers //
		FileIO::ReadString(file, token);	// <Layer>:
		int layerSize = FileIO::ReadVal<int>(file);
		for (int i = 0; i < layerSize; ++i) {
			layer->AddLayer(LoadAnimatorLayer(file));
		}

		return layer;
	}
}
















namespace FileIO {
	void ReadString(FILE* file, std::string& token)
	{
		constexpr int kTokenBuffSize = 256;

		BYTE length{};
		token.resize(kTokenBuffSize);

		::fread(&length, sizeof(BYTE), 1, file);
		UINT nReads = (UINT)::fread(token.data(), sizeof(char), length, file);

		if (nReads == 0) {
			throw std::runtime_error("Failed to read a file!\n");
		}

		token.resize(length);
	}


	sptr<MasterModel> LoadGeometryFromFile(const std::string& filePath)
	{
		FILE* file = nullptr;
		::fopen_s(&file, filePath.c_str(), "rb");
		::rewind(file);

		sptr<Model> model = std::make_shared<Model>();
		sptr<MasterModel> masterModelA = std::make_shared<MasterModel>();
		sptr<AnimationLoadInfo> animationInfo{};
		std::string token;

		bool isEOF{ false };
		while (!isEOF) {
			FileIO::ReadString(file, token);

			switch (Hash(token)) {
			case Hash("<Controller>:"):
				animationInfo = std::make_shared<AnimationLoadInfo>();
				FileIO::LoadAnimation(file, animationInfo);
				break;

			case Hash("<Hierarchy>:"):
				model = ::LoadFrameHierarchy(file, animationInfo);
				break;
			case Hash("</Hierarchy>"):
				isEOF = true;
				break;

			default:
				assert(0);
				break;
			}
		}

		sptr<MasterModel> masterModel = std::make_shared<MasterModel>();
		masterModel->SetModel(model);

		if (animationInfo) {
			masterModel->SetAnimationInfo(animationInfo);
		}

		return masterModel;
	}


	void LoadAnimation(FILE* file, sptr<AnimationLoadInfo>& animationInfo)
	{
		FileIO::ReadString(file, animationInfo->AnimatorControllerFile);
	}


	sptr<AnimationClip> LoadAnimationClip(const std::string& filePath)
	{
		FILE* file = nullptr;
		::fopen_s(&file, filePath.c_str(), "rb");
		::rewind(file);

		std::string clipName;
		FileIO::ReadString(file, clipName); //Animation Set Name

		float length    = FileIO::ReadVal<float>(file);
		int frameRate   = FileIO::ReadVal<int>(file);
		int keyFrameCnt = FileIO::ReadVal<int>(file);
		int boneCnt		= FileIO::ReadVal<int>(file);

		sptr<AnimationClip> clip = std::make_shared<AnimationClip>(length, frameRate, keyFrameCnt, boneCnt, clipName);

		for (int i = 0; i < keyFrameCnt; ++i) {
			FileIO::ReadVal(file, clip->mKeyFrameTimes[i]);
			FileIO::ReadRange(file, clip->mKeyFrameTransforms[i], boneCnt);
		}

		return clip;
	}

	sptr<AnimatorController> LoadAnimatorController(const std::string& filePath)
	{
		std::string token{};
		FILE* file = nullptr;
		::fopen_s(&file, filePath.c_str(), "rb");
		::rewind(file);

		// Params //
		Animations::ParamMap params{};

		FileIO::ReadString(file, token);	// <Params>:
		int paramSize = FileIO::ReadVal<int>(file);

		for (int i = 0; i < paramSize; ++i) {
			AnimatorParameter param{};
			std::string paramName;
			FileIO::ReadString(file, paramName);

			// Set param type and default value
			std::string paramType;
			FileIO::ReadString(file, paramType);
			switch (Hash(paramType)) {
			case Hash("Float"):
				param.type = AnimatorParameter::Type::Float;
				FileIO::ReadVal(file, param.val.f);
				break;
			case Hash("Int"):
				param.type = AnimatorParameter::Type::Int;
				FileIO::ReadVal(file, param.val.i);
				break;
			case Hash("Bool"):
				param.type = AnimatorParameter::Type::Bool;
				FileIO::ReadVal(file, param.val.b);
				break;
			case Hash("Trigger"):
				param.type = AnimatorParameter::Type::Trigger;
				param.val.b = false;
				break;
			default:
				assert(0);
				break;
			}

			params.insert(std::make_pair(paramName, param));
		}

		// Layer //
		sptr<AnimatorLayer> baseLayer = ::LoadAnimatorLayer(file);

		return std::make_shared<AnimatorController>(params, baseLayer);
	}

	void LoadLightFromFile(const std::string& filePath, LightInfo** out)
	{
		LightInfo* light = *out;

		FILE* file = nullptr;
		::fopen_s(&file, filePath.c_str(), "rb");
		assert(file);
		::rewind(file);

		std::string token{};
		std::string name{};

		light->Ambient = Vec4(0.f, 0.f, 0.f, 1.f);

		FileIO::ReadString(file, token); // <Type:>
		FileIO::ReadVal(file, light->Type);

		FileIO::ReadString(file, token); // <Color:>
		FileIO::ReadVal(file, light->Diffuse);

		float intensity;
		FileIO::ReadString(file, token); // <Intensity:>
		FileIO::ReadVal(file, intensity);

		FileIO::ReadString(file, token); // <Range:>
		FileIO::ReadVal(file, light->Range);

		float angle{};
		FileIO::ReadString(file, token); // <Angle:>
		FileIO::ReadVal(file, angle);

		angle       /= 2.0f; // 보정값
		light->Phi   = static_cast<float>(cos(XMConvertToRadians(angle)));
		light->Theta = static_cast<float>(cos(XMConvertToRadians(angle / 2.0f)));

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

		factor.z  = factor.y * 0.01f;
		factor.x  = factor.y * 100.f;
		factor.x  = min(1.f, factor.x);

		light->Attenuation = Vec3(factor.x, factor.y, factor.z); // 감쇠
		light->Falloff     = 1.f;
		light->Specular    = Vec4(0.1f, 0.1f, 0.1f, 1.f);
	}

	std::unordered_map<std::string, sptr<Texture>> LoadTextures(const std::string& folder)
	{
		std::unordered_map<std::string, sptr<Texture>> result{};

		// get [textureNames] from [folder]
		std::vector<std::string> textureNames{};
		GetTextureNames(textureNames, folder);

		// load textures
		for (auto& textureName : textureNames) {
			FileIO::RemoveExtension(textureName);

			sptr<Texture> texture = std::make_shared<Texture>(D3DResource::Texture2D);
			texture->LoadTexture(folder, textureName);

			result.insert(std::make_pair(textureName, texture));
		}

		return result;
	}
}