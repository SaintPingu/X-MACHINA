#include "stdafx.h"
#include "NewtorkTimeManager.h"


DEFINE_SINGLETON(NewtorkTimeManager);


NewtorkTimeManager::NewtorkTimeManager()
{
}

NewtorkTimeManager::~NewtorkTimeManager()
{

}

long long NewtorkTimeManager::GetTimeStamp()
{
	return duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
