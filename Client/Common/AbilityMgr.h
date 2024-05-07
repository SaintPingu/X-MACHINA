#pragma once


#pragma region ClassForwardDecl
class Mesh;
class Shader;
class GameObject;
class Object;
#pragma endregion


#pragma region Ability
class Ability {
protected:
	// TODO : 외부에서 선언하거나 내부에서 선언하거나
	Object* mObject{};
	float mCooldownTime{};
	float mActiveTime{};

public:
	Ability(float cooldownTime, float activeTime) : mCooldownTime(cooldownTime), mActiveTime(activeTime) {}

public:
	float GetCooldownTime() const { return mCooldownTime; }
	float GetActiveTime() const { return mActiveTime; }

	void SetObject(Object* object) { mObject = object; }

public:
	virtual void Update() abstract;
	virtual void Activate() {};
	virtual void DeActivate() {};
};

class RenderedAbility : public Ability, public std::enable_shared_from_this<RenderedAbility> {
protected:
	D3D_PRIMITIVE_TOPOLOGY mPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	sptr<GameObject> mRenderedObject;
	sptr<Mesh> mRenderedMesh;

	sptr<Shader> mShader{};

public:
	RenderedAbility(float cooldownTime, float activeTime) : Ability(cooldownTime, activeTime) {}

public:
	virtual void Update() override;
	virtual void Activate() override;
	virtual void DeActivate() override;
	
public:
	virtual void Render();
};
#pragma endregion


#pragma region AbilityMgr
class AbilityMgr : public Singleton<AbilityMgr> {
	friend Singleton;

private:
	std::unordered_set<sptr<RenderedAbility>> mRenderedAbilities;

public:
	void AddRenderedAbilities(sptr<RenderedAbility> ability) { mRenderedAbilities.insert(ability); }
	void RemoveRenderedAbilities(sptr<RenderedAbility> ability) { mRenderedAbilities.erase(ability); }

	void Render();
};
#pragma endregion

