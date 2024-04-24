#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
#pragma endregion


#pragma region Class
class Collider abstract : public Component {
	COMPONENT_ABSTRACT(Collider, Component)

public:
	virtual void Render() const {};

	virtual bool Intersects(rsptr<Collider> other) abstract;
	virtual bool Intersects(const BoundingBox& bb) abstract;
	virtual bool Intersects(const BoundingSphere& bs) abstract;
	virtual bool Intersects(const Ray& ray, float& dist) abstract;

	enum class Type { None, Box, Sphere };
	virtual Type GetType() const abstract;
};




// basic cuboid-shaped collision primitive.
class BoxCollider : public Collider {
	COMPONENT(BoxCollider, Collider)

public:
	MyBoundingOrientedBox mBox{};

public:
	BoxCollider& operator=(const BoxCollider& other);

public:
	virtual void Update() override;
	virtual void Render() const override;

	virtual bool Intersects(rsptr<Collider> other) override;
	virtual bool Intersects(const BoundingBox& bb) override;
	virtual bool Intersects(const BoundingSphere& bs) override;
	virtual bool Intersects(const Ray& ray, float& dist) override;

	virtual Type GetType() const { return Type::Box; }
};




// basic sphere-shaped collision primitive.
class SphereCollider : public Collider {
	COMPONENT(SphereCollider, Collider)

public:
	MyBoundingSphere mBS{};

public:
	SphereCollider& operator=(const SphereCollider& other);

public:
	virtual void Update();
	virtual void Render() const override;

	virtual bool Intersects(rsptr<Collider> other) override;
	virtual bool Intersects(const BoundingBox& bb) override;
	virtual bool Intersects(const BoundingSphere& bs) override;
	virtual bool Intersects(const Ray& ray, float& dist) override;

	virtual Type GetType() const { return Type::Sphere; }
};



// for collision check
class ObjectCollider : public Component {
	COMPONENT(ObjectCollider, Component)

private:
	sptr<SphereCollider>		mSphereCollider{};	// (��ü ��ü�� ���δ�)SphereCollider�� �ݵ�� �־�� �ϸ� �ϳ��� �����ؾ� �Ѵ�.
	std::vector<sptr<Collider>>	mColliders{};		// ��ü collider

public:
	const MyBoundingSphere& GetBS() const { return mSphereCollider->mBS; }
	const std::vector<sptr<Collider>>& GetColliders() const { return mColliders; }

public:
	virtual void Awake() override;
	virtual void Update() override;

public:
	void Render() const;

	bool Intersects(const ObjectCollider* other) const;
	bool Intersects(const BoundingFrustum& frustum) const { return frustum.Intersects(mSphereCollider->mBS); }
	bool Intersects(const BoundingBox& bb) const;
	bool Intersects(const BoundingSphere& bs) const;

	// �� ��ü�� �浹 ���θ� ��ȯ�Ѵ�.
	static bool Intersects(const GridObject& a, const GridObject& b);
};
#pragma endregion