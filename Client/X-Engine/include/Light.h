#pragma once

#pragma region ClassForwardDecl
class ModelObjectMesh;
class GameObject;
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
	LightInfo* GetLight(int index) const { return &mLights->Lights[index]; }
	// ��ü ������ ��ȯ�Ѵ�.
	rsptr<SceneLight> GetSceneLights() const { return mLights; }
	UINT GetLightCount() const { return static_cast<UINT>(mLights->Lights.size()); }

	const Matrix& GetLightViewMtx() const { return mMtxLightView; }
	const Matrix& GetLightProjMtx() const { return mMtxLightProj; }
	const Matrix& GetShadowMtx() const { return mMtxShadow; }

public:
	// ���ο� ���� ���� �����Ѵ�.
	void InsertLightModel(const std::string& name, const LightLoadInfo* light) { mLightModels.insert(std::make_pair(name, light)); }

	void SetSceneBounds(float boundRadius);
	void BuildLights(std::ifstream& file);
	void BuildLights();

	void Update();
	void UpdateShaderVars(int index);

	// ������ ���� �޽��� �������Ѵ�.
	void Render();

	// ������ Sunlight�� �����Ѵ�.
	void SetSunlight();

private:

	// ���� �� �������� ���� �𵨵��� �ҷ��´�.
	void LoadLightModels();
	// [file]���� ���� ���� �ҷ����� �Ҵ��Ѵ�.
	void LoadLightObjects(std::ifstream& file);
};
#pragma endregion