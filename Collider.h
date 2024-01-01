#pragma once

#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
#pragma endregion


#pragma region Class
class Collider : public Component {
	COMPONENT_ABSTRACT(Collider, Component)
};




// basic cuboid-shaped collision primitive.
class BoxCollider : public Collider {
	COMPONENT(BoxCollider, Collider)

public:
	MyBoundingOrientedBox mBox;

public:
	BoxCollider& operator=(const BoxCollider& other);

public:
	virtual void Update();
	virtual void Render() const;
};




// basic sphere-shaped collision primitive.
class SphereCollider : public Collider {
	COMPONENT(SphereCollider, Collider)

public:
	MyBoundingSphere mBS;

public:
	SphereCollider& operator=(const SphereCollider& other);

public:
	virtual void Update();
	virtual void Render() const;
};



// for collision check
class ObjectCollider : public Component {
	COMPONENT(ObjectCollider, Component)

private:
	std::shared_ptr<SphereCollider>		mSphereCollider{};	// (객체 전체를 감싸는)SphereCollider가 반드시 있어야 하며 하나만 존재해야 한다.
	std::vector<MyBoundingOrientedBox*> mOBBList{};			// 전체 bounding box
	std::vector<sptr<BoxCollider>>		mBoxColliders{};	// 전체 box collider

public:
	const MyBoundingSphere& GetBS() const { return mSphereCollider->mBS; }
	const auto& GetOBBList() const { return mOBBList; }

public:
	virtual void Start() override;
	virtual void Update() override;

public:
	void Render() const;

	bool Intersects(const BoundingFrustum& frustum) const { return frustum.Intersects(mSphereCollider->mBS); }

	// bs를 각 obb에 대해 교차 여부를 반환한다.
	static inline bool Intersects(const std::vector<MyBoundingOrientedBox*>& obbList, const BoundingSphere& bs)
	{
		for (const auto& boxA : obbList) {
			if (boxA->Intersects(bs)) {
				return true;
			}
		}

		return false;
	}

	// 두 obb list에 대해 각 요소의 교차 여부를 반환한다.
	static inline bool Intersects(const std::vector<MyBoundingOrientedBox*>& obbListA, const std::vector<MyBoundingOrientedBox*>& obbListB)
	{
		for (const auto& obbA : obbListA) {
			for (const auto& obbB : obbListB) {
				if (obbA->Intersects(*obbB)) {
					return true;
				}
			}
		}

		return false;
	}

	// 두 객체의 충돌 여부를 반환한다.
	static bool Intersects(const GameObject& a, const GameObject& b);
};
#pragma endregion