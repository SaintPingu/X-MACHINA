#include "EnginePch.h"
#include "LobbyScene.h"

#include "Object.h"
#include "ObjectPool.h"
#include "Model.h"
#include "Light.h"
#include "SkyBox.h"
#include "ResourceMgr.h"
#include "DXGIMgr.h"
#include "Shader.h"
#include "FileIO.h"

#include "Component/Camera.h"
#include "Component/UI.h"



void LobbyScene::RenderShadow()
{
	base::RenderShadow();

	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	RESOURCE<Shader>("Shadow_Global")->Set();
	for (const auto& object : mMeshObjects) {
		object->Render();
	}

	RESOURCE<Shader>("Shadow_SkinMesh")->Set();
	for (const auto& object : mSkinMeshObjects) {
		object->Render();
	}

	RESOURCE<Shader>("Shadow_ObjectInst")->Set();
	for (auto& buffer : mObjectPools) {
		buffer->PushRenderAllObjects();
		buffer->Render();
	}
}

void LobbyScene::RenderDeferred()
{
	base::RenderDeferred();

	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	RenderObjects();
}

void LobbyScene::RenderForward()
{
	base::RenderForward();

	RESOURCE<Shader>("SkyBox")->Set();
	mSkyBox->Render();
}

void LobbyScene::Update()
{
	//ParticleManager::I->Update();
	UpdateObjects();
	mLight->Update();

	MainCamera::I->Update();
	Canvas::I->Update();

	UpdateShaderVars();

	mManager->Update();
}

void LobbyScene::Build()
{
	base::Build();

	std::cout << "Load Lobby Scene...";
	mLight->SetSceneBounds(15.f);

	LoadSceneObjects();

	Start();
	std::cout << "OK\n";
}

void LobbyScene::Release()
{
	base::Release();

	mMeshObjects.clear();
	mSkinMeshObjects.clear();
	mObjectPools.clear();
}

GameObject* LobbyScene::Instantiate(const std::string& modelName, const Vec3& pos)
{
	const auto& model = RESOURCE<MasterModel>(modelName);
	if (!model) {
		return nullptr;
	}

	sptr<GameObject> instance = std::make_shared<GameObject>();
	instance->SetPosition(Vector3::Zero);
	instance->SetModel(model);

	if (instance->IsSkinMesh()) {
 		mSkinMeshObjects.push_back(instance);
	}
	else {
		mMeshObjects.push_back(instance);
	}

	return instance.get();
}


void LobbyScene::Start()
{
	ProcessAllObjects([](sptr<Object> object) {
		object->Awake();
		});
	mManager->Awake();

	MainCamera::I->SetActive(true);
	ProcessAllObjects([](sptr<Object> object) {
		object->SetActive(true);
		});
	mManager->SetActive(true);
}

void LobbyScene::UpdateObjects()
{
	ProcessAllObjects([](sptr<Object> object) {
		object->Update();
		});

	ProcessAllObjects([](sptr<Object> object) {
		object->Animate();
		});

	ProcessAllObjects([](sptr<Object> object) {
		object->LateUpdate();
		});
}

void LobbyScene::RenderObjects()
{
	RESOURCE<Shader>("Global")->Set();
	for (const auto& object : mMeshObjects) {
		object->Render();
	}

	RESOURCE<Shader>("SkinMesh")->Set();
	for (const auto& object : mSkinMeshObjects) {
		object->Render();
	}

	RESOURCE<Shader>("ObjectInst")->Set();
	for (auto& buffer : mObjectPools) {
		buffer->PushRenderAllObjects();
		buffer->Render();
	}
}

void LobbyScene::LoadSceneObjects()
{
	const std::string kFileName = "Import/LobbyScene.bin";
	std::ifstream file = FileIO::OpenBinFile(kFileName);

	LoadGameObjects(file);
}

void LobbyScene::LoadGameObjects(std::ifstream& file)
{
	std::string token{};
	std::string name{};

	FileIO::ReadString(file, token); // "<Lights>:"
	FileIO::ReadVal<int>(file);

	int objectCount;
	FileIO::ReadString(file, token); // "<GameObjects>:"
	FileIO::ReadVal(file, objectCount);

	mMeshObjects.reserve(objectCount);
	mSkinMeshObjects.reserve(objectCount);

	int sameObjectCount{};			// get one unique model from same object
	sptr<MasterModel> model{};
	sptr<ObjectPool> objectPool{};

	bool isInstancing{};
	ObjectTag tag{};

	for (int i = 0; i < objectCount; ++i) {
		sptr<GameObject> object{};

		if (sameObjectCount <= 0) {
			FileIO::ReadString(file, token); //"<Tag>:"
			FileIO::ReadString(file, token);

			FileIO::ReadString(file, token); //"<MeshName>:"
			std::string meshName = FileIO::ReadString(file);
			model = RESOURCE<MasterModel>(meshName);

			FileIO::ReadString(file, token); //"<Transforms>:"
			FileIO::ReadVal(file, sameObjectCount);

			FileIO::ReadString(file, token); //"<IsInstancing>:"
			FileIO::ReadVal(file, isInstancing);

			if (isInstancing) {
				objectPool = CreateObjectPool(model, sameObjectCount);
			}
		}

		if (sameObjectCount > 0) {
			if (isInstancing) {
				object = objectPool->Get(false);
				objectPool->SetStatic();
			}
			else {
				object = std::make_shared<GameObject>();
				object->SetModel(model);

				if (object->IsSkinMesh()) {
					mSkinMeshObjects.push_back(object);
				}
				else {
					mMeshObjects.push_back(object);
				}
			}

			// transform
			{
				FileIO::ReadString(file, token);	// <Transform>:

				Matrix transform;
				FileIO::ReadVal(file, transform);
				object->SetWorldTransform(transform);

				FileIO::ReadString(file, token);	// </Transform>:

				--sameObjectCount;
			}
		}
	}
}

sptr<ObjectPool> LobbyScene::CreateObjectPool(rsptr<const MasterModel> model, int maxSize, const std::function<void(rsptr<InstObject>)>& objectInitFunc)
{
	sptr<ObjectPool> pool = mObjectPools.emplace_back(std::make_shared<ObjectPool>(model, maxSize));
	pool->CreateObjects<InstObject>(objectInitFunc);

	return pool;
}

void LobbyScene::ProcessAllObjects(const std::function<void(sptr<GameObject>)>& processFunc)
{
	for (auto& object : mMeshObjects) {
		processFunc(object);
	}

	for (auto& object : mSkinMeshObjects) {
		processFunc(object);
	}

	for (auto& object : mObjectPools) {
		object->DoAllObjects(processFunc);
	}
}

void LobbyScene::ProcessActiveObjects(const std::function<void(sptr<GameObject>)>& processFunc)
{
	for (auto& object : mMeshObjects) {
		if (object->IsActive()) {
			processFunc(object);
		}
	}

	for (auto& object : mSkinMeshObjects) {
		if (object->IsActive()) {
			processFunc(object);
		}
	}

	for (auto& object : mObjectPools) {
		object->DoActiveObjects(processFunc);
	}
}