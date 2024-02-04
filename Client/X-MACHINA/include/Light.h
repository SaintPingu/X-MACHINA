#pragma once

#pragma region EnumClass
// must be matched with Light.hlsl LightType
enum class LightType {
	Spot        = 0,
	Directional = 1,
	Point       = 2
};
#pragma endregion


#pragma region Variable
// move to stdafx.h
#pragma endregion


#pragma region Struct
// move to stdafx.h
#pragma endregion


#pragma region Class
class Light {
private:
	sptr<SceneLight> mLights{};	// all lights in scene
	std::set<std::string> mLightModelNames{};

	size_t					mCurrLightCnt{};	// count of allocated light in scene

	std::unordered_map<std::string, const LightInfo*> mLightModels{};	// �ϳ��� ���� �𵨿� ���� ���� ������ �������� ���� ������ �� ���

public:
	Light();
	virtual ~Light();

	// [modelName]�� ���� LightInfo�� ��ȯ�Ѵ�.
	const LightInfo* GetLightModel(const std::string& modelName) const;
	// [index]�� ���� LightInfo�� ��ȯ�Ѵ�.
	LightInfo* GetLight(int index) const { return &mLights->Lights[index]; }
	// ��ü ������ ��ȯ�Ѵ�.
	rsptr<SceneLight> GetSceneLights() const { return mLights; }

public:

	// ���ο� ���� ���� �����Ѵ�.
	void InsertLightModel(const std::string& name, const LightInfo* light) { mLightModels.insert(std::make_pair(name, light)); }

	void BuildLights(FILE* file);

private:

	// ������ Sunlight�� �����Ѵ�.
	void SetSunlight();

	// ���� �� �������� ���� �𵨵��� �ҷ��´�.
	void LoadLightModels();
	// [file]���� ���� ���� �ҷ����� �Ҵ��Ѵ�.
	void LoadLightObjects(FILE* file);
};
#pragma endregion