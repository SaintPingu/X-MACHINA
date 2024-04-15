#include "EnginePch.h"
#include "Resources.h"

Resource::Resource(ResourceType resourceType)
	:
	mResourceType(resourceType)
{
}

Resource::Resource(const Resource& other)
	:
	mResourceType(other.mResourceType),
	mName(other.mName)
{
}
