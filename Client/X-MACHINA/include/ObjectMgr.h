#pragma once

#define objectMgr ObjectMgr::Inst()

class ObjectMgr : public Singleton<ObjectMgr> {
	friend Singleton;

public:
	void InitObjectsScript();
};