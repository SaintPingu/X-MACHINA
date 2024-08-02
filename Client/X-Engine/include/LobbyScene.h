#pragma once

#include "Scene.h"

class UI;
class GameObject;
class ObjectPool;
class MasterModel;
class InstObject;

class LobbyScene : public Singleton<LobbyScene>, public Scene {
	friend Singleton;

public:
	std::vector<sptr<GameObject>> mMeshObjects{};
	std::vector<sptr<GameObject>> mSkinMeshObjects{};
	std::vector<sptr<ObjectPool>> mObjectPools{};

public:
	virtual void RenderBegin() override;

	virtual void RenderShadow() override;
	virtual void RenderDeferred() override;
	virtual void RenderCustomDepth() override;
	virtual void RenderForward() override;

	virtual void ApplyDynamicContext() override;

	void Update();
	virtual void Build() override;
	virtual void Release() override;

	GameObject* Instantiate(const std::string& modelName, const Vec3& pos = Vector3::Zero);


private:
	void Start();
	void BuildText();
	void UpdateObjects();
	void RenderObjects();

	void LoadSceneObjects();
	void LoadGameObjects(std::ifstream& file);

	sptr<ObjectPool> CreateObjectPool(rsptr<const MasterModel> model, int maxSize, const std::function<void(rsptr<InstObject>)>& objectInitFunc = nullptr);

	void ProcessAllObjects(const std::function<void(sptr<GameObject>)>& processFunc);
	void ProcessActiveObjects(const std::function<void(sptr<GameObject>)>& processFunc);
};
