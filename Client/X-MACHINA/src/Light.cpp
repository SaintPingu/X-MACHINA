#include "stdafx.h"
#include "Light.h"

#include "Scene.h"
#include "FileIO.h"

namespace {
	constexpr int gkSunLightIdx = 0;

	std::set<std::string> gkLightModelNames = { "apache_high_light", "tank_head_light", "tank_high_light" };
}




Light::Light()
	:
	mLights(std::make_shared<SceneLight>())
{
	XMStoreFloat4(&mLights->FogColor, Colors::Gray);
}

Light::~Light()
{
	for (auto& light : mLightModels) {
		if (gkLightModelNames.contains(light.first)) {
			delete light.second;
		}
	}

	gkLightModelNames.clear();
}

const LightInfo* Light::GetLightModel(const std::string& modelName) const
{
	assert(mLightModels.contains(modelName));

	return mLightModels.at(modelName);
}

void Light::BuildLights(FILE* file)
{
	LoadLightObjects(file);
	LoadLightModels();

	mLights->Lights[gkSunLightIdx].IsEnable = true;	// sunlight(전역조명)를 활성화한다.
	SetSunlight();
}

void Light::SetSunlight()
{
	LightInfo& light     = mLights->Lights[gkSunLightIdx];
	light.Type           = static_cast<int>(LightType::Directional);
	light.Ambient        = Vec4(0.1f, 0.1f, 0.1f, 1.f);
	light.Diffuse        = Vec4(1.f, 0.956f, 0.839f, 1.f);
	light.Specular       = Vec4(0.5f, 0.5f, 0.5f, 1.f);
	light.Direction      = Vec3(-0.6f, -0.6f, 0.3f);
}

void Light::LoadLightModels()
{
	for (auto& name : gkLightModelNames) {
		LightInfo* light = new LightInfo;
		FileIO::LoadLightFromFile("Models/Lights/" + name + ".bin", &light);
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
	LightInfo* modelLight{};
	for (size_t i = 1; i < lightCount; ++i) {
		LightInfo* light = &mLights->Lights[i];
		light->IsEnable  = true;

		if (sameLightCount <= 0) {
			FileIO::ReadString(file, token); //"<FileName>:"

			std::string fileName{};
			FileIO::ReadString(file, fileName);

			FileIO::LoadLightFromFile("Models/Lights/" + fileName + ".bin", &light);
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
