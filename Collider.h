#pragma once

#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class Object;
#pragma endregion


#pragma region Class
class Collider : public Component {
	COMPONENT_ABSTRACT(Collider, Component)
};





class BoxCollider : public Collider {
	COMPONENT(BoxCollider, Collider)

public:
	MyBoundingOrientedBox mBox;

public:
	virtual void Update();
	virtual void Render() const;
};





class SphereCollider : public Collider {
	COMPONENT(SphereCollider, Collider)

public:
	MyBoundingSphere mBS;

public:
	virtual void Update();
	virtual void Render() const;
};





class ObjectCollider : public Component {
	COMPONENT(ObjectCollider, Component)

private:
	bool mIsCollidable{};

	std::shared_ptr<SphereCollider>		mSphereCollider{};
	std::vector<MyBoundingOrientedBox*> mOBBList{};
	std::vector<sptr<BoxCollider>>		mBoxColliders{};

public:
	ObjectCollider& operator=(const ObjectCollider& other);

	MyBoundingSphere& GetBS() const { return mSphereCollider->mBS; }
	const auto& GetOBBList() const { return mOBBList; }

	void SetCollidable(bool collidable) { mIsCollidable = collidable; }

public:
	virtual void Start() override;
	virtual void Update() override;

	bool IsCollidable() const { return mIsCollidable; }

	void Render() const;

	bool Intersects(const BoundingSphere& sphere) const;
	bool Intersects(const BoundingBox& box) const;
	bool IntersectsBS(const BoundingSphere& sphere) const { return mSphereCollider->mBS.Intersects(sphere); }
	bool Intersects(const BoundingOrientedBox& box) const;
	bool Intersects(const std::vector<MyBoundingOrientedBox*>& boxes) const;
	bool IsInFrustum(const BoundingFrustum& frustum) const { return frustum.Intersects(mSphereCollider->mBS); }
	bool Intersects(const Object& other) const;

private:
	void UpdateCollidable();
};
#pragma endregion