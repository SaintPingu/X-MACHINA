#pragma once

#include "Scene.h"

class UI;
class GameObject;
class ObjectPool;
class MasterModel;
class InstObject;

class LobbyScene : public Singleton<LobbyScene>, public Scene {
	friend Singleton;
	using base = Scene;

public:
	std::vector<sptr<GameObject>> mMeshObjects{};
	std::vector<sptr<GameObject>> mSkinMeshObjects{};
	std::vector<sptr<ObjectPool>> mObjectPools{};

public:
	LobbyScene();
	~LobbyScene() = default;

public:
	virtual void RenderShadow() override;
	virtual void RenderDeferred() override;
	virtual void RenderForward() override;

	virtual void Update() override;
	virtual void Build() override;
	virtual void Release() override;

	GameObject* Instantiate(const std::string& modelName, const Vec3& pos = Vector3::Zero);
	void RemoveSkinMeshObject(GameObject* target);

private:
	void Start();
	void UpdateObjects();
	void RenderObjects();

	void LoadSceneObjects();
	void LoadGameObjects(std::ifstream& file);

	sptr<ObjectPool> CreateObjectPool(rsptr<const MasterModel> model, int maxSize, const std::function<void(rsptr<InstObject>)>& objectInitFunc = nullptr);

	void ProcessAllObjects(const std::function<void(sptr<GameObject>)>& processFunc);
	void ProcessActiveObjects(const std::function<void(sptr<GameObject>)>& processFunc);
};
