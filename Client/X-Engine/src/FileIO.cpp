#include "EnginePch.h"
#include "FileIO.h"
#include "DXGIMgr.h"

#include "BattleScene.h"
#include "Model.h"
#include "Mesh.h"
#include "Light.h"
#include "Component/Collider.h"
#include "Texture.h"
#include "Animator.h"
#include "AnimationClip.h"
#include "AnimatorMotion.h"
#include "AnimatorStateMachine.h"
#include "AnimatorController.h"
#include "AnimatorLayer.h"
#include "ResourceMgr.h"


namespace {
	namespace {	// Model IO

		// Mesh 정보를 불러온다. (Positions, Normals, ...)
		sptr<MeshLoadInfo> LoadMesh(std::ifstream& file)
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



		sptr<SkinMesh> LoadSkinMesh(std::ifstream& file, sptr<MeshLoadInfo>& meshInfo, int& skinBoneCnt)
		{
			sptr<SkinMesh> mesh = std::make_shared<SkinMesh>();
			std::string token;

			bool isEOF = false;
			while (!isEOF) {
				FileIO::ReadString(file, token);

				switch (Hash(token)) {
				case Hash("<BoneNames>:"):
				{
					sptr<Avatar> avatar;

					FileIO::ReadString(file, token);
					if (token == "<Avatar>:") {
						avatar = std::make_shared<Avatar>();
						int avatarBoneLength = FileIO::ReadVal<int>(file);

						std::string boneName;
						std::string boneType;
						for (int i = 0; i < avatarBoneLength; ++i) {
							FileIO::ReadString(file, boneName);
							FileIO::ReadString(file, boneType);
							avatar->SetBoneType(boneName, boneType);
						}

						FileIO::ReadString(file, token); // <Names>:
					}

					int cnt = FileIO::ReadVal<int>(file);
					if (cnt > 0) {
						mesh->mBoneNames.resize(cnt);

						for (int i = 0; i < cnt; i++)
						{
							FileIO::ReadString(file, mesh->mBoneNames[i]);
								mesh->mBoneNameIndices[mesh->mBoneNames[i]] = skinBoneCnt++;
							if (avatar) {
								const std::string& boneName = mesh->mBoneNames[i];
								mesh->mBoneTypes[boneName] = avatar->GetBoneType(boneName);
							}
						}
					}
				}

				break;
				case Hash("<BoneOffsets>:"):
				{
					int skinBoneCnt = FileIO::ReadVal<int>(file);
					if (skinBoneCnt > 0) {
						std::vector<Matrix> boneOffsets(skinBoneCnt);
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
		std::vector<sptr<Material>> LoadMaterial(std::ifstream& file)
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

				Vec4 temp1 = Vec4(0.f, 0.f, 0.f, 0.f); // TODO : 삭제

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
				case Hash("<EmissionColor>:"):
					FileIO::ReadVal(file, matInfo->Emission);
					break;
				case Hash("<AlbedoColor>:"):
					FileIO::ReadVal(file, matInfo->DiffuseAlbedo);
					break;
				case Hash("<Metallic>:"):
					FileIO::ReadVal(file, matInfo->Metallic);
					break;
				case Hash("<Roughness>:"):
					FileIO::ReadVal(file, matInfo->Roughness);
					break;

				case Hash("<EmissiveColor>:"):
					FileIO::ReadVal(file, temp1);
					break;

				case Hash("<SpecularColor>:"):
					FileIO::ReadVal(file, temp1);
					break;

				case Hash("<Glossiness>:"):
					float temp2; // TODO : 삭제
					FileIO::ReadVal(file, temp2);
					break;

				case Hash("<Smoothness>:"):
					FileIO::ReadVal(file, matInfo->Roughness);
					break;

				case Hash("<SpecularHighlight>:"):
					FileIO::ReadVal(file, temp2);
					break;

				case Hash("<GlossyReflection>:"):
					FileIO::ReadVal(file, temp2);
					break;

					// 여러 텍스처를 로드할 수 있도록 변경
				case Hash("<AlbedoMap>:"):
					material->LoadTextureFromFile(TextureMap::DiffuseMap0, file);
					break;

				case Hash("<NormalMap>:"):
					material->LoadTextureFromFile(TextureMap::NormalMap, file);
					break;

				case Hash("<EmissionMap>:"):
					material->LoadTextureFromFile(TextureMap::EmissiveMap, file);
					break;

				case Hash("<MetallicMap>:"):
					material->LoadTextureFromFile(TextureMap::MetallicMap, file);
					break;

				case Hash("<OcclusionMap>:"):
					material->LoadTextureFromFile(TextureMap::OcclusionMap, file);
					break;

				case Hash("<OcclusionMask>:"):
					FileIO::ReadVal(file, matInfo->OcclusionMask);
					break;

				case Hash("<AlphaTest>"):
					matInfo->AlphaTest = true;
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
		sptr<Model> LoadFrameHierarchy(std::ifstream& file, sptr<AnimationLoadInfo>& animationInfo, int& skinBoneCnt)
		{
			std::string token;

			int nFrame = 0;
			int nTextures = 0;

			sptr<Model> model = std::make_shared<Model>();
			sptr<MeshLoadInfo> meshInfo{};

			FileIO::ReadString(file, token);	// <Name>:
			model->SetName(FileIO::ReadString(file));

			FileIO::ReadString(file, token);	// <TransformMatrix>:
			Matrix transform = FileIO::ReadVal<Matrix>(file);
			model->SetLocalTransform(transform);

			bool isEOF{ false };
			while (!isEOF) {
				FileIO::ReadString(file, token);

				switch (Hash(token)) {
				case Hash("<BoundingSphere>:"):
				{
					int sphereCnt = FileIO::ReadVal<int>(file);

					std::vector<Vec3> centers;
					std::vector<float> radiuses;
					FileIO::ReadRange(file, centers, sphereCnt);
					FileIO::ReadRange(file, radiuses, sphereCnt);

					MyBoundingSphere bs{};
					for (int i = 0; i < sphereCnt; ++i) {
						bs.SetOrigin(centers[i]);
						bs.Center = centers[i];
						bs.Radius = radiuses[i];

						model->AddComponent<SphereCollider>()->mBS = bs;
					}
				}

				break;
				case Hash("<BoundingBoxes>:"):
				{
					int boxCnt = FileIO::ReadVal<int>(file);

					std::vector<Vec3> centers;
					std::vector<Vec3> extents;
					FileIO::ReadRange(file, centers, boxCnt);
					FileIO::ReadRange(file, extents, boxCnt);

					MyBoundingOrientedBox box{};
					for (int i = 0; i < boxCnt; ++i) {
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
					meshInfo->SkinMesh = ::LoadSkinMesh(file, meshInfo, skinBoneCnt);
					if (!animationInfo) {
						animationInfo = std::make_shared<AnimationLoadInfo>();
					}
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
						std::vector<sptr<Model>> childs{};
						for (int i = 0; i < nChilds; i++) {
							childs.push_back(LoadFrameHierarchy(file, animationInfo, skinBoneCnt));
						}
						for(const auto& child : childs){
							model->SetChild(child);
						}
					}
				}

				break;
				case Hash("</Frame>"):
					isEOF = true;
					break;

				default:
					assert(0);
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
	}

	namespace { // Animation IO
		AnimatorTransition LoadTransition(std::ifstream& file)
		{
			AnimatorTransition transition;

			FileIO::ReadString(file, transition.Destination);
			int conditionSize = FileIO::ReadVal<int>(file);
			transition.Conditions.resize(conditionSize);

			for (auto& condition : transition.Conditions) {
				FileIO::ReadString(file, condition.mode);
				FileIO::ReadString(file, condition.paramName);

				switch (Hash(condition.mode)) {
				case Hash("If"):
				case Hash("IfNot"):
					break;
				default:
					FileIO::ReadVal(file, condition.threshold);
					break;
				}
			}

			return transition;
		}

		std::vector<AnimatorTransition> LoadTransitions(std::ifstream& file)
		{
			std::vector<AnimatorTransition> transitions{};

			int transitionSize = FileIO::ReadVal<int>(file);
			transitions.resize(transitionSize);

			for (int i = 0; i < transitionSize; ++i) {
				transitions[i] = LoadTransition(file);
			}

			return transitions;
		}

		std::vector<AnimatorMotionTransition> LoadStateTransitions(std::ifstream& file)
		{
			std::vector<AnimatorMotionTransition> transitions{};

			int transitionSize = FileIO::ReadVal<int>(file);
			transitions.resize(transitionSize);

			for (int i = 0; i < transitionSize; ++i) {
				LoadTransition(file);
				FileIO::ReadVal(file, transitions[i].ExitTime);
			}

			return transitions;
		}

		HumanBone LoadAvatarMask(std::ifstream& file)
		{
			HumanBone result = HumanBone::None;
			result |= FileIO::ReadVal<bool>(file) ? HumanBone::Root : HumanBone::None;
			result |= FileIO::ReadVal<bool>(file) ? HumanBone::Body : HumanBone::None;
			result |= FileIO::ReadVal<bool>(file) ? HumanBone::LeftArm : HumanBone::None;
			result |= FileIO::ReadVal<bool>(file) ? HumanBone::RightArm : HumanBone::None;
			result |= FileIO::ReadVal<bool>(file) ? HumanBone::LeftLeg : HumanBone::None;
			result |= FileIO::ReadVal<bool>(file) ? HumanBone::RightLeg : HumanBone::None;
			result |= FileIO::ReadVal<bool>(file) ? HumanBone::Head : HumanBone::None;
			result |= FileIO::ReadVal<bool>(file) ? HumanBone::LeftHand : HumanBone::None;
			result |= FileIO::ReadVal<bool>(file) ? HumanBone::RightHand : HumanBone::None;

			return result;
		}

		sptr<const AnimationClip> ReadAnimationClip(std::ifstream& file)
		{
			std::string clipFolder = FileIO::ReadString(file);
			std::string clipName = FileIO::ReadString(file);
			return RESOURCE<AnimationClip>(clipFolder + '/' + clipName);
		}

		sptr<BlendTree> LoadAnimatorBlendTree(std::ifstream& file, AnimatorMotionInfo& motionInfo)
		{
			FileIO::ReadString(file, motionInfo.Name);

			int motionCnt = FileIO::ReadVal<int>(file);
			std::vector<sptr<ChildMotion>> childMotions;
			childMotions.reserve(motionCnt);

			sptr<const AnimationClip> clip{};
			for (int i = 0; i < motionCnt; ++i) {
				clip = ReadAnimationClip(file);
				Vec2 position = FileIO::ReadVal<Vec2>(file);
				childMotions.push_back(std::make_shared<ChildMotion>(clip, position));
			}

			return std::make_shared<BlendTree>(motionInfo, childMotions);
		}

		sptr<AnimatorState> LoadAnimatorState(std::ifstream& file, AnimatorMotionInfo& motionInfo)
		{
			sptr<const AnimationClip> clip = ReadAnimationClip(file);
			if (!clip) {
				throw std::runtime_error("[Error] Couldn't read animation clip");
			}

			return std::make_shared<AnimatorState>(motionInfo, clip);
		}

		sptr<AnimatorStateMachine> LoadAnimatorStateMachine(std::ifstream& file)
		{
			std::string token;

			///// Default State Machine Info /////
			std::string stateMachineName = FileIO::ReadString(file);

			// Entry //
			FileIO::ReadString(file, token);	// <Entry>:
			std::vector<AnimatorTransition> entryTransitions = LoadTransitions(file);

			sptr<AnimatorStateMachine> stateMachine = std::make_shared<AnimatorStateMachine>(stateMachineName, entryTransitions);
			//////////////////////////////////


			// Motions //
			FileIO::ReadString(file, token);	// <Motions>:
			int stateSize = FileIO::ReadVal<int>(file);

			AnimatorMotionInfo motionInfo{};

			for (int i = 0; i < stateSize; ++i) {

				sptr<AnimatorMotion> motion{};

				FileIO::ReadVal<float>(file, motionInfo.Speed);
				LoadStateTransitions(file);

				std::string motionType = FileIO::ReadString(file);

				switch (Hash(motionType)) {
				case Hash("<BlendTree>:"):
					motion = LoadAnimatorBlendTree(file, motionInfo);
					break;
				case Hash("<State>:"):
					motion = LoadAnimatorState(file, motionInfo);
					break;
				default:
					assert(0);
					break;
				}

				stateMachine->AddState(motion);
			}

			// Sub State Machines //
			FileIO::ReadString(file, token);	// <SubStateMachines>:
			int layerSize = FileIO::ReadVal<int>(file);
			for (int i = 0; i < layerSize; ++i) {
				stateMachine->AddStateMachine(LoadAnimatorStateMachine(file));
			}

			return stateMachine;
		}

		sptr<AnimatorLayer> LoadAnimatorLayer(std::ifstream& file)
		{
			std::string token;

			std::string layerName;
			FileIO::ReadString(file, layerName);

			HumanBone boneMask = HumanBone::All;
			FileIO::ReadString(file, token);	// <AvatarMask>: or <StateMachines>:
			if (token == "<AvatarMask>:") {
				boneMask = LoadAvatarMask(file);

				FileIO::ReadString(file, token);	// <StateMachines>:
			}

			sptr<AnimatorStateMachine> rootStateMachine = LoadAnimatorStateMachine(file);

			return std::make_shared<AnimatorLayer>(layerName, rootStateMachine, boneMask);
		}
	}
}


































namespace FileIO {
	namespace ModelIO {
		sptr<MasterModel> LoadGeometryFromFile(const std::string& filePath)
		{
			std::ifstream file = OpenBinFile(filePath);

			sptr<Model> model = std::make_shared<Model>();
			sptr<MasterModel> masterModelA = std::make_shared<MasterModel>();
			sptr<AnimationLoadInfo> animationInfo{};
			std::string token;
			int skinBoneCnt{};

			bool isEOF{ false };
			while (!isEOF) {
				FileIO::ReadString(file, token);

				switch (Hash(token)) {
				case Hash("<Controller>:"):
					animationInfo = std::make_shared<AnimationLoadInfo>();
					FileIO::AnimationIO::SetAnimation(file, animationInfo);
					FileIO::ReadVal(file, animationInfo->IsManualBoneCalc);
					break;

				case Hash("<Hierarchy>:"):
					model = ::LoadFrameHierarchy(file, animationInfo, skinBoneCnt);
					break;

				case Hash("<ScriptExporter>:"):
					BattleScene::I->LoadScriptExporter(file, model);
					break;

				case Hash("</Hierarchy>"):
					isEOF = true;
					break;

				default:
					assert(0);
					break;
				}
			}

			assert(skinBoneCnt < gkSkinBoneSize);

			sptr<MasterModel> masterModel = std::make_shared<MasterModel>();
			masterModel->SetModel(model);

			if (animationInfo) {
				masterModel->SetAnimationInfo(animationInfo);
			}

			return masterModel;
		}

		void LoadLightFromFile(const std::string& filePath, LightLoadInfo** out)
		{
			LightLoadInfo* light = *out;

			std::ifstream file = OpenBinFile(filePath);

			std::string token{};

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

			angle /= 2.0f; // 보정값
			light->Phi = static_cast<float>(cos(XMConvertToRadians(angle)));
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

			factor.z = factor.y * 0.01f;
			factor.x = factor.y * 100.f;
			factor.x = min(1.f, factor.x);

			light->Attenuation = Vec3(factor.x, factor.y, factor.z); // 감쇠
			light->Falloff = 1.f;
			light->Specular = Vec4(0.1f, 0.1f, 0.1f, 1.f);
		}

		void LoadTextures(const std::string& folder, D3DResource textureType)
		{
			// get [textureNames] from [folder]
			std::vector<std::string> textureNames{};
			GetTextureNames(textureNames, folder);

			// load textures
			for (auto& textureName : textureNames) {
				FileIO::RemoveExtension(textureName);

				sptr<Texture> texture = std::make_shared<Texture>(textureType);
				texture->LoadTexture(textureName, folder);
				ResourceMgr::I->Add<Texture>(textureName, texture);
			}
		}
	}

	namespace AnimationIO {
		void SetAnimation(std::ifstream& file, sptr<AnimationLoadInfo>& animationInfo)
		{
			FileIO::ReadString(file, animationInfo->AnimatorControllerFile);
		}


		sptr<AnimationClip> LoadAnimationClip(const std::string& filePath)
		{
			std::ifstream file = OpenBinFile(filePath);

			std::string clipName = FileIO::ReadString(file); //Animation Set Name

			const float length = FileIO::ReadVal<float>(file);
			const int frameRate = FileIO::ReadVal<int>(file);
			const int boneCnt = FileIO::ReadVal<int>(file);
			const int keyFrameCnt = static_cast<int>(ceil(length * frameRate));

			sptr<AnimationClip> clip = std::make_shared<AnimationClip>(length, frameRate, keyFrameCnt, boneCnt, clipName);

			clip->mKeyFrameTimes.resize(keyFrameCnt);
			for (int i = 0; i < keyFrameCnt; ++i) {
				FileIO::ReadVal(file, clip->mKeyFrameTimes[i]);

				for (int j = 0; j < boneCnt; ++j) {
					std::string boneName = FileIO::ReadString(file);
					auto& keyFrameTransforms = clip->mKeyFrameTransforms[boneName];
					Matrix matrix = FileIO::ReadVal<Matrix>(file);
					clip->mKeyFrameTransforms[boneName].push_back(matrix);
				}
			}

			return clip;
		}

		sptr<AnimatorController> LoadAnimatorController(const std::string& filePath)
		{
			std::string token{};
			std::ifstream file = OpenBinFile(filePath);

			// Params //
			Animations::ParamMap params{};

			FileIO::ReadString(file, token);	// <Params>:
			int paramSize = FileIO::ReadVal<int>(file);

			for (int i = 0; i < paramSize; ++i) {
				AnimatorParameter param{};
				std::string paramName = FileIO::ReadString(file);

				// Set param type and default value
				std::string paramType = FileIO::ReadString(file);
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
				default:
					assert(0);
					break;
				}

				params.insert(std::make_pair(paramName, param));
			}

			// Layer //
			std::vector<sptr<AnimatorLayer>> layers;

			FileIO::ReadString(file, token);	// <Layers>:
			int layerCnt = FileIO::ReadVal<int>(file);
			layers.resize(layerCnt);
			for (auto& layer : layers) {
				layer = ::LoadAnimatorLayer(file);
			}

			return std::make_shared<AnimatorController>(params, layers);
		}
	}
}