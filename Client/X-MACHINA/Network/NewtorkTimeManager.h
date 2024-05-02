#pragma once

#define NETWORK_TIME_MGR NewtorkTimeManager::GetInst()

class NewtorkTimeManager
{
	DECLARE_SINGLETON(NewtorkTimeManager);

public:
	NewtorkTimeManager();
	~NewtorkTimeManager();

public:
	long long GetTimeStamp();

};

