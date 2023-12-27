#pragma once
#include "Component.h"

class Object;
class Mesh;
class Camera;





class Collider : public Component {
	COMPONENT_ABSTRACT(Component, Collider)
};


class BoxCollider : public Collider {
	COMPONENT(Collider, BoxCollider)

public:
	MyBoundingOrientedBox mBox;

	virtual void Update();
	virtual void Render() const;
};

class SphereCollider : public Collider {
	COMPONENT(Collider, SphereCollider)

public:
	MyBoundingSphere mBS;

	virtual void Update();
	virtual void Render() const;
};






class ObjectCollider : public Component {
	COMPONENT(Component, ObjectCollider)

private:
	bool mIsCollidable{};

	std::shared_ptr<SphereCollider> mSphereCollider{};
	std::vector<MyBoundingOrientedBox*> mOBBList{};
	std::vector<sptr<BoxCollider>> mBoxColliders{};

public:
	ObjectCollider& operator=(const ObjectCollider& other) {
		if (this == &other) {
			return *this;
		}

		mIsCollidable = other.mIsCollidable;
		mSphereCollider = other.mSphereCollider;

		return *this;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Getter ] /////

public:
	bool IsCollidable() const { return mIsCollidable; }
	MyBoundingSphere& GetBS() const { return mSphereCollider->mBS; }

	const std::vector<MyBoundingOrientedBox*>& GetOBBList() const { return mOBBList; }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Setter ] /////

	void SetCollidable(bool collidable) { mIsCollidable = collidable; }


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Others ] /////
private:
	void UpdateCollidable();

public:
	virtual void Start() override;
	virtual void Update() override;

	virtual void Render() const;

	/* Intersects */
	virtual bool Intersects(const BoundingSphere& sphere) const;
	virtual bool Intersects(const BoundingBox& box) const;
	virtual bool IntersectsBS(const BoundingSphere& sphere) const;
	virtual bool Intersects(const BoundingOrientedBox& box) const;
	virtual bool Intersects(const std::vector<BoundingOrientedBox>& boxes) const;
	virtual bool IsInFrustum(const BoundingFrustum& frustum) const;
	virtual bool Intersects(const Object& other) const;
};