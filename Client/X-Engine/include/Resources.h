#pragma once

enum class ResourceType : UINT8 {
	None = 0,
	Texture,
	Mesh,
	Model,
	Shader,
	AnimationClip,
	AnimatorController,
	ParticleSystemCPUData,
	ParticleSystemGPUData,
	AnimatorMotion,

	_count
};

enum {
	ResourceTypeCount = static_cast<UINT8>(ResourceType::_count)
};

class Resource {
protected:
	int mID{ -1 };
	ResourceType mResourceType = ResourceType::None;
	std::string mName;

#pragma region C/Dtor
public:
	Resource(ResourceType resourceType);
	Resource(const Resource& other);
	virtual ~Resource();
#pragma endregion

#pragma region Getter
public:
	ResourceType GetResourceType() const { return mResourceType; }
	const std::string& GetName() const { return mName; }
	int GetID() const { return mID; }
#pragma endregion

#pragma region Setter
	void SetName(const std::string& name) { mName = name; }
#pragma endregion

protected:
	friend class ResourceMgr;
};


class ResourceMemoryLeakChecker : public Singleton<ResourceMemoryLeakChecker> {
	friend Singleton;

private:
	static bool mIsActive;
	int mNewID{};
	std::unordered_map<int, Resource*> mResources{};

public:
	virtual ~ResourceMemoryLeakChecker() { SetActive(false); }

public:
	int Add(Resource* resource);
	void Del(int id);

	static void SetActive(bool val) { mIsActive = val; }
	static bool IsActive() { return mIsActive; }
	void Report();
};