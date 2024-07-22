#include "EnginePch.h"
#include "Resources.h"

#include "AnimatorController.h"

Resource::Resource(ResourceType resourceType)
	:
	mResourceType(resourceType)
{
#if defined(_DEBUG)
	if (ResourceMemoryLeakChecker::IsActive()) {
		mID = ResourceMemoryLeakChecker::I->Add(this);
	}
#endif
}

Resource::Resource(const Resource& other)
	:
	mResourceType(other.mResourceType),
	mName(other.mName)
{
#if defined(_DEBUG)
	if (ResourceMemoryLeakChecker::IsActive()) {
		mID = ResourceMemoryLeakChecker::I->Add(this);
	}
#endif
}

Resource::~Resource()
{
#if defined(_DEBUG)
	if (ResourceMemoryLeakChecker::IsActive()) {
		ResourceMemoryLeakChecker::I->Del(mID);
	}
#endif
}


bool ResourceMemoryLeakChecker::mIsActive = false;
int ResourceMemoryLeakChecker::Add(Resource* resource)
{
	mResources[mNewID] = resource;
	if (mNewID >= 118) {
		int a = 5;
	}
	return mNewID++;
}

void ResourceMemoryLeakChecker::Del(int id)
{
	if (mResources.count(id)) {
		mResources.erase(id);
	}
}

void ResourceMemoryLeakChecker::Report()
{
#if defined(_DEBUG)
	if (!mResources.empty()) {
		std::cout << "==========[Leak Resources]==========\n";
		for (const auto& [id, resource] : mResources) {
			ResourceType resourceType = resource->GetResourceType();
			std::cout << "(" << resource->GetID() << ")Name [ " << resource->GetName() << " ], Type [ " << static_cast<int>(resourceType) << " ]\n";
		}
		mResources.clear();
		std::cout << "====================================\n";
	}
#endif
}
