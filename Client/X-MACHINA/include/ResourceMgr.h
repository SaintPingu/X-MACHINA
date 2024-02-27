#pragma once

#pragma region Include
#include "Resources.h"
#pragma endregion

#pragma region Define
#define res ResourceMgr::Inst()
#pragma endregion

#pragma region ClassForwardDecl
class Texture;
class Shader;
class ModelObjectMesh;
#pragma endregion

class ResourceMgr : public Singleton<ResourceMgr> {
	friend Singleton;

private:
	using KeyResMap = std::unordered_map<std::string, sptr<Resource>>;
	std::array<KeyResMap, ResourceTypeCount> mResources;

public:
#pragma region C/Dtor
	ResourceMgr() = default;
	virtual ~ResourceMgr() = default;
#pragma endregion

#pragma region Template
public:
	template<typename T>
	sptr<T> Load(const std::string& key, const std::string& path);

	template<typename T>
	bool Add(const std::string& key, sptr<T> resource);

	template<typename T>
	sptr<T> Get(const std::string& Key);

	template<typename T>
	ResourceType GetResourceType();
#pragma endregion

public:
	void Init();
	void Clear();
	sptr<Texture> CreateTexture(const std::string& name, UINT width, UINT height, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS resourcecFlags, D3D12_RESOURCE_STATES resourceStates, Vec4 clearColor = Vec4());
	sptr<Texture> CreateTexture(const std::string& name, ComPtr<ID3D12Resource> resource);
	sptr<ModelObjectMesh> LoadRectangleMesh();

private:
	void LoadTextures();
	void LoadShaders();
};

template<typename T>
inline sptr<T> ResourceMgr::Load(const std::string& key, const std::string& path)
{
	ResourceType resourceType = GetResourceType<T>();
	KeyResMap& keyResMap = mResources[static_cast<UINT8>(resourceType)];

	auto findIt = keyResMap.find(key);
	if (findIt != keyResMap.end()) {
		return std::static_pointer_cast<T>(findIt->second);
	}

	sptr<T> resource = std::make_shared<T>();
	resource->Load(key, path);
	keyResMap[key] = resource;
	keyResMap[key]->SetName(key);

	return resource;
}

template<typename T>
inline bool ResourceMgr::Add(const std::string& key, sptr<T> resource)
{
	ResourceType resourceType = GetResourceType<T>();
	KeyResMap& keyResMap = mResources[static_cast<UINT8>(resourceType)];

	auto findIt = keyResMap.find(key);
	if (findIt != keyResMap.end()) {
		return false;
	}

	keyResMap[key] = resource;
	keyResMap[key]->SetName(key);

	return true;
}

template<typename T>
inline sptr<T> ResourceMgr::Get(const std::string& key)
{
	ResourceType resourceType = GetResourceType<T>();
	KeyResMap& keyResMap = mResources[static_cast<UINT8>(resourceType)];

	auto findIt = keyResMap.find(key);
	if (findIt != keyResMap.end()) {
		return std::static_pointer_cast<T>(findIt->second);
	}

	return nullptr;
}

template<typename T>
inline ResourceType ResourceMgr::GetResourceType()
{
	if (std::is_same_v<T, Texture>)
		return ResourceType::Texture;
	if (std::is_same_v<T, Shader>)
		return ResourceType::Shader;
	if ( std::is_same_v<T, ModelObjectMesh>)
		return ResourceType::Mesh;
}
