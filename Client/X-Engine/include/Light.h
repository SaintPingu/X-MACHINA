#pragma once

#pragma region ClassForwardDecl
class ModelObjectMesh;
class GameObject;
struct SceneLight;
struct SceneLoadLight;
struct LightLoadInfo;
struct LightInfo;
#pragma endregion


#pragma region EnumClass
// must be matched with Light.hlsl LightType
enum class LightType {
	Spot        = 0,
	Directional = 1,
	Point       = 2
};
#pragma endregion


#pragma region Class
class Light {
private:
	sptr<SceneLight>		mLights{};			// all lights in scene
	sptr<SceneLoadLight>	mLoadLights{};		// all load lights in scene
	size_t					mCurrLightCnt{};	// count of allocated light in scene
	
	float					mSceneBoundsRadius{};
	BoundingSphere			mSceneBounds{};		// �þ� ��ü ���� �ش� ��ü �κп��� ����ȴ�.
	Matrix					mMtxLightView{};
	Matrix					mMtxLightProj{};
	Matrix					mMtxShadow{};

	std::unordered_map<std::string, const LightLoadInfo*> mLightModels{};	// �ϳ��� ���� �𵨿� ���� ���� ������ �������� ���� ������ �� ���
	std::set<std::string> mLightModelNames{};

public:
	Light();
	virtual ~Light();

	// [modelName]�� ���� LightInfo�� ��ȯ�Ѵ�.
	const LightLoadInfo* GetLightModel(const std::string& modelName) const;

	// [index]�� ���� LightInfo�� ��ȯ�Ѵ�.
	LightInfo* GetLight(int index) const;
	// ��ü ������ ��ȯ�Ѵ�.
	rsptr<SceneLight> GetSceneLights() const { return mLights; }
	UINT GetLightCount() const;

	const Matrix& GetLightViewMtx() const { return mMtxLightView; }
	const Matrix& GetLightProjMtx() const { return mMtxLightProj; }
	const Matrix& GetShadowMtx() const { return mMtxShadow; }

public:
	// ���ο� ���� ���� �����Ѵ�.
	void InsertLightModel(const std::string& name, const LightLoadInfo* light) { mLightModels.insert(std::make_pair(name, light)); }

	float GetSceneBoundRadius() const { return mSceneBoundsRadius; }
	void SetSceneBounds(float boundRadius);

	void Update();
	void UpdateShaderVars(int index);

	// ������ ���� �޽��� �������Ѵ�.
	void Render();

	void SetSunlightDir(const Vec3&);
	void SetSunlightColor(const Vec3& color);

private:

	// ������ Sunlight�� �����Ѵ�.
	void SetSunlight();
	void BuildLights();
	// ���� �� �������� ���� �𵨵��� �ҷ��´�.
	void LoadLightModels();
	// [file]���� ���� ���� �ҷ����� �Ҵ��Ѵ�.
	void LoadLightObjects(std::ifstream& file);
};
#pragma endregion