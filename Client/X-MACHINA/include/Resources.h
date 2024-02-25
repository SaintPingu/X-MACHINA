#pragma once

enum class ResourceType : UINT8 {
	None,
	Texture,

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
	virtual ~Resource() = default;
#pragma endregion

#pragma region Getter
public:
	ResourceType GetResourceType() const { return mResourceType; }
	const std::string& GetName() const { return mName; }
#pragma endregion

#pragma region Setter
	void SetName(std::string name) { mName = name; }
#pragma endregion

protected:
	friend class ResourceMgr;
	virtual void Load(const std::string& name, const std::string& path) { }
};

