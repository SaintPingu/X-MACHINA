#include "stdafx.h"
#include "Light.h"

#include "Scene.h"
#include "FileIO.h"
#include "Mesh.h"
#include "ResourceMgr.h"

namespace {
	constexpr int gkSunLightIdx = 0;
}

Light::Light()
	:
	mLights(std::make_shared<SceneLight>()),
	mLoadLights(std::make_shared<SceneLoadLight>())
{
	mLightModelNames = { "apache_high_light", "tank_head_light", "tank_high_light" };
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

	mLoadLights->Lights[gkSunLightIdx].IsEnable = true;	// sunlight(전역조명)를 활성화한다.
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

	}
}

void Light::Update()
{
}

void Light::Render()
{
	res->Get<ModelObjectMesh>("Rect")->Render();
}

void Light::SetSunlight()
{
	LightLoadInfo& light     = mLoadLights->Lights[gkSunLightIdx];
	light.Type           = static_cast<int>(LightType::Directional);
	light.Ambient        = Vec4(0.1f, 0.1f, 0.1f, 1.f);
	light.Diffuse        = Vec4(0.9f, 0.9f, 0.9f, 1.f);
	light.Specular       = Vec4(0.5f, 0.5f, 0.5f, 1.f);
	light.Direction		 = Vec3(0.57735f, -0.57735f, 0.57735f);
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
