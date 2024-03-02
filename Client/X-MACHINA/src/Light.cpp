#include "stdafx.h"
#include "Light.h"

#include "DXGIMgr.h"
#include "FrameResource.h"
#include "ResourceMgr.h"
#include "Scene.h"
#include "FileIO.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "Scene.h"
#include "Object.h"

namespace {
	constexpr int gkSunLightIdx = 0;
}

Light::Light()
	:
	mLights(std::make_shared<SceneLight>()),
	mLoadLights(std::make_shared<SceneLoadLight>())
{
	mLightModelNames = { "apache_high_light", "tank_head_light", "tank_high_light" };

	mSceneBounds.Center = Vec3(0.f, 0.f, 0.f);
	mSceneBounds.Radius = sqrt(5.f * 5.f + 5.f * 5.f);
}

Light::~Light()
{
	for (auto& light : mLightModels) {
		if (mLightModelNames.contains(light.first)) {
			delete light.second;
		}
	}

	mLightModelNames.clear();
}

const LightLoadInfo* Light::GetLightModel(const std::string& modelName) const
{
	assert(mLightModels.contains(modelName));

	return mLightModels.at(modelName);
}

void Light::BuildLights(FILE* file)
{
	LoadLightObjects(file);
	LoadLightModels();

	SetSunlight();

	BuildLights();
}

void Light::BuildLights()
{
	// TODO : 현재는 로드 라이트를 라이트에 복사하지만, 로드 부분을 수정하여 라이트에 바로 데이터를 채워줄 예정이다.
	for (int i = 0; i < mLoadLights->Lights.size(); ++i) {
		const auto& loadLight = mLoadLights->Lights[i];
		auto& light = mLights->Lights[i];
		light.Strength = Vec3{ loadLight.Diffuse.x, loadLight.Diffuse.y, loadLight.Diffuse.z };
		light.FalloffStart = 1.f;
		light.Direction = loadLight.Direction;
		light.FalloffEnd = 30.f;
		light.Position = loadLight.Position;
		light.SpotPower = 64.f;
		light.Type = loadLight.Type;
		light.IsEnable = loadLight.IsEnable;

		auto& volumeMesh = mLights->VolumeMeshes[i];
		switch (static_cast<LightType>(light.Type))
		{
		case LightType::Spot:
		case LightType::Point:
			volumeMesh = std::make_shared<ModelObjectMesh>();
			volumeMesh->CreateSphereMesh(light.FalloffEnd);
			break;
		case LightType::Directional:
			volumeMesh = res->Get<ModelObjectMesh>("Rect");
			break;
		}
	}
}

void Light::Update()
{
	auto& sunLight = mLights->Lights[gkSunLightIdx];
	mSceneBounds.Center = scene->GetPlayer()->GetPosition();

	// 태양 조명 뷰 행렬 생성
	Vec3 lightDir = sunLight.Direction;
	Vec3 lightPos = -2.f * mSceneBounds.Radius * lightDir;
	Vec3 targetPos = mSceneBounds.Center;
	Vec3 lightUp = Vec3::Up;
	Matrix lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);
	sunLight.Position = lightPos;
	
	// 바운딩 구 센터를 조명 좌표계로 변환
	Vec3 sphereCenterLS = Vec3::Transform(targetPos, lightView);
	
	// 조명 좌표계에서의 직교 프러스텀 생성
	float l = sphereCenterLS.x - mSceneBounds.Radius;
	float b = sphereCenterLS.y - mSceneBounds.Radius;
	float n = sphereCenterLS.z - mSceneBounds.Radius;
	float r = sphereCenterLS.x + mSceneBounds.Radius;
	float t = sphereCenterLS.y + mSceneBounds.Radius;
	float f = sphereCenterLS.z + mSceneBounds.Radius;

	Matrix lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// NDC 공간을 텍스처 공간으로 변환
	Matrix mtxTexture(
		0.5f, 0.f, 0.f, 0.f,
		0.f, -0.5f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.5f, 0.5f, 0.f, 1.f);

	Matrix mtxShadow = lightView * lightProj * mtxTexture;
	mMtxLightView = lightView;
	mMtxLightProj = lightProj;
}

void Light::UpdateShaderVars(int index)
{
	// TODO : 동적 조명만 업데이트
	auto& light = mLights->Lights[index];
	ObjectConstants objectConstants;
	objectConstants.MtxWorld = XMMatrixTranslation(light.Position.x, light.Position.y, light.Position.z);
	objectConstants.LightIndex = index;
	frmResMgr->CopyData(light.ObjCBIndex, objectConstants);
	dxgi->SetGraphicsRootConstantBufferView(RootParam::Object, frmResMgr->GetObjCBGpuAddr(light.ObjCBIndex));
}

void Light::Render()
{
	for (int i = 0; i < mLights->Lights.size(); ++i) {
		auto& light = mLights->Lights[i];
		auto& volumeMesh = mLights->VolumeMeshes[i];
		if (light.IsEnable) {
			switch (static_cast<LightType>(light.Type))
			{
			case LightType::Spot:
			case LightType::Point:
				res->Get<Shader>("SpotPointLighting")->Set();
				break;
			case LightType::Directional:
				res->Get<Shader>("DirLighting")->Set();
				break;
			default:
				break;
			}
			
			UpdateShaderVars(i);
			volumeMesh->Render();
		}
	}
}

void Light::SetSunlight()
{
	LightLoadInfo& light = mLoadLights->Lights[gkSunLightIdx];
	light.Type           = static_cast<int>(LightType::Directional);
	light.Ambient        = Vec4(0.1f, 0.1f, 0.1f, 1.f);
	light.Diffuse        = Vec4(0.9f, 0.9f, 0.9f, 1.f);
	light.Specular       = Vec4(0.5f, 0.5f, 0.5f, 1.f);
	light.Direction		 = Vec3(0.57735f, -0.57735f, 0.57735f);
	light.IsEnable		 = true;
}

void Light::LoadLightModels()
{
	for (auto& name : mLightModelNames) {
		LightLoadInfo* light = new LightLoadInfo;
		FileIO::LoadLightFromFile("Import/Lights/" + name + ".bin", &light);
		mLightModels.insert(std::make_pair(name, light));
	}
}

void Light::LoadLightObjects(FILE* file)
{
	std::string token{};
	std::string name{};

	int lightCount;
	FileIO::ReadString(file, token); // "<Lights>:"
	FileIO::ReadVal(file, lightCount);

	++lightCount;
	assert(lightCount <= gkMaxSceneLight);

	int sameLightCount{};
	LightLoadInfo* modelLight{};
	for (size_t i = 1; i < lightCount; ++i) {
		LightLoadInfo* light = &mLoadLights->Lights[i];
		light->IsEnable  = true;

		if (sameLightCount <= 0) {
			FileIO::ReadString(file, token); //"<FileName>:"

			std::string fileName{};
			FileIO::ReadString(file, fileName);

			FileIO::LoadLightFromFile("Import/Lights/" + fileName + ".bin", &light);
			InsertLightModel(fileName, light);

			modelLight = light;

			FileIO::ReadString(file, token); //"<Transforms>:"
			FileIO::ReadVal(file, sameLightCount);
		}

		*light = *modelLight;

		Vec4x4 transform;
		FileIO::ReadVal(file, transform);

		light->Position.x = transform._41;
		light->Position.y = transform._42;
		light->Position.z = transform._43;

		light->Direction.x = transform._31;
		light->Direction.y = transform._32;
		light->Direction.z = transform._33;

		--sameLightCount;
	}
}
