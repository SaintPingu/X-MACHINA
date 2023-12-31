#pragma once

#pragma region EnumClass
enum class LightType {
	Spot = 0,
	Directional = 1,
	Point = 2,
	Area = 3,
	Rectangle = 3,
	Disc = 4
};

#pragma endregion


#pragma region Variable
constexpr int gkMaxSceneLight = 32;
#pragma endregion


#pragma region Struct
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

struct SceneLight {
	LightInfo Lights[gkMaxSceneLight]{};

	Vec4	GlobalAmbient{};

	Vec4	FogColor{};
	float	FogStart{ 100.f };
	float	FogRange{ 300.f };
};

struct LightInfoRange {
	sptr<SceneLight>* Lights{};

	size_t	Begin{};
	size_t End{};

	LightInfo& GetLight(size_t i) { return (*Lights)->Lights[i]; }
};
#pragma endregion


#pragma region Class
class Light {
private:
	sptr<SceneLight> mLights{};

	ComPtr<ID3D12Resource>	mCBLights{};
	size_t					mCurrLightCnt{};
	SceneLight*					mCBMap_Lights{};

	std::unordered_map<std::string, const LightInfo*> mLightModels;

public:
	Light();
	virtual ~Light();

	const LightInfo* GetLightModel(const std::string& modelName) const;
	LightInfo* GetLight(int index) const { return &mLights->Lights[index]; }

public:
	void InsertLight(std::string name, const LightInfo* light) { mLightModels.insert(std::make_pair(name, light)); }

	void BuildLights(FILE* file);

	LightInfoRange AlloLight(size_t count);

	void CreateShaderVars();
	void UpdateShaderVars();
	void ReleaseShaderVars();

private:
	void SetSunlight();

	void LoadLightModels();
	void LoadLightObjects(FILE* file);
};
#pragma endregion