#pragma once

class GameObject;

class ObjectMgr : public Singleton<ObjectMgr> {
	friend Singleton;

private:
	std::vector<sptr<GameObject>> mObjects;

public:
	void InitObjectsScript();
	std::vector<sptr<GameObject>> GetAllObjects() const { return mObjects; }
};