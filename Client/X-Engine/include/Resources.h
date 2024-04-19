#pragma once

enum class ResourceType : UINT8 {
	None,
	Texture,
	Mesh,
	Model,
	Shader,
	AnimationClip,
	AnimatorController,
	ParticleSystemCPUData,

	_count
};

enum {
	ResourceTypeCount = static_cast<UINT8>(ResourceType::_count)
};

class Resource {
protected:
	ResourceType mResourceType = ResourceType::None;
	std::string mName;

#pragma region C/Dtor
public:
	Resource(ResourceType resourceType);
	Resource(const Resource& other);
	virtual ~Resource() = default;
#pragma endregion

#pragma region Getter
public:
	ResourceType GetResourceType() const { return mResourceType; }
	const std::string& GetName() const { return mName; }
#pragma endregion

#pragma region Setter
	void SetName(const std::string& name) { mName = name; }
#pragma endregion

protected:
	friend class ResourceMgr;
};

