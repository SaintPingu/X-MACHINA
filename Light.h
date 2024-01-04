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
constexpr int gkMaxSceneLight = 32;	// 씬에 존재할 수 있는 조명의 최대 개수. Light.hlsl과 동일해야 한다.
#pragma endregion


#pragma region Struct
// must be matched with Light.hlsl LightInfo
struct LightInfo {
	Vec4	Ambient{};
	Vec4	Diffuse{};
	Vec4	Specular{};

	Vec3	Position{};
	float	Falloff{};

	Vec3	Direction{};
	float	Theta{};

	Vec3	Attenuation{};
	float	Phi{};

	float	Range{};
	float	Padding{};
	int		Type{};
	bool	IsEnable{};
};

// must be matched with Light.hlsl cbLights
struct SceneLight {
	std::array<LightInfo, gkMaxSceneLight> Lights{};

	Vec4	GlobalAmbient{};

	Vec4	FogColor{};
	float	FogStart = 100.f;
	float	FogRange = 300.f;
};
#pragma endregion


#pragma region Class
class Light {
private:
	sptr<SceneLight> mLights{};	// all lights in scene

	ComPtr<ID3D12Resource>	mCB_Lights{};
	SceneLight*				mCBMap_Lights{};
	size_t					mCurrLightCnt{};	// count of allocated light in scene

	std::unordered_map<std::string, const LightInfo*> mLightModels{};	// 하나의 조명 모델에 대해 여러 조명을 동적으로 생성 가능한 모델 목록

public:
	Light();
	virtual ~Light();

	// [modelName]에 따른 LightInfo을 반환한다.
	const LightInfo* GetLightModel(const std::string& modelName) const;
	// [index]에 따른 LightInfo를 반환한다.
	LightInfo* GetLight(int index) const { return &mLights->Lights[index]; }

public:

	// 새로운 조명 모델을 삽입한다.
	void InsertLightModel(const std::string& name, const LightInfo* light) { mLightModels.insert(std::make_pair(name, light)); }

	void BuildLights(FILE* file);

	void CreateShaderVars();
	void UpdateShaderVars();
	void ReleaseShaderVars();

private:

	// 조명을 Sunlight로 설정한다.
	void SetSunlight();

	// 조명 모델 폴더에서 조명 모델들을 불러온다.
	void LoadLightModels();
	// [file]에서 조명 모델을 불러오고 할당한다.
	void LoadLightObjects(FILE* file);
};
#pragma endregion