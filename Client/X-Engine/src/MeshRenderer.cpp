#include "EnginePch.h"
#include "MeshRenderer.h"

#include "Mesh.h"
#include "Component/Transform.h"



uptr<ModelObjectMesh> MeshRenderer::mBoxMesh;
uptr<ModelObjectMesh> MeshRenderer::mSphereMesh;
uptr<ModelObjectMesh> MeshRenderer::mPlaneMesh;

void MeshRenderer::Render(const BoundingBox& box)
{
	const Vec3 center  = box.Center;
	const Vec3 extents = box.Extents;

	const XMMATRIX scaleMtx       = XMMatrixScaling(extents.x * 2, extents.y * 2, extents.z * 2);
	const XMMATRIX translationMtx = XMMatrixTranslation(center.x, center.y, center.z);

	const XMMATRIX matrix = XMMatrixMultiply(scaleMtx, translationMtx);		// (Scale * Translate)

	Transform::UpdateColliderShaderVars(matrix);
	mBoxMesh->Render();
}

void MeshRenderer::Render(const BoundingOrientedBox& box)
{
	const Vec3 center         = box.Center;
	const Vec3 extents        = box.Extents;
	const Vec4 orientation    = box.Orientation;
	const XMVECTOR quaternion = XMLoadFloat4(&orientation);

	const XMMATRIX scaleMtx       = XMMatrixScaling(extents.x * 2, extents.y * 2, extents.z * 2);
	const XMMATRIX rotationMtx    = XMMatrixRotationQuaternion(quaternion);
	const XMMATRIX translationMtx = XMMatrixTranslation(center.x, center.y, center.z);
	
	const XMMATRIX matrix = XMMatrixMultiply(XMMatrixMultiply(scaleMtx, rotationMtx), translationMtx);	// (Scale * Rotation) * Translate

	Transform::UpdateColliderShaderVars(matrix);
	mBoxMesh->Render();
}

void MeshRenderer::Render(const BoundingSphere& bs)
{
	const Vec3 center  = bs.Center;
	const float radius = bs.Radius;

	const XMMATRIX scaleMtx       = XMMatrixScaling(radius, radius, radius);
	const XMMATRIX translationMtx = XMMatrixTranslation(center.x, center.y, center.z);

	const XMMATRIX matrix = XMMatrixMultiply(scaleMtx, translationMtx);	// (Scale * Translate)

	Transform::UpdateColliderShaderVars(matrix);
	mSphereMesh->Render();
}

void MeshRenderer::RenderPlane(const Vec3& pos, float width, float length)
{
	const XMMATRIX scaleMtx       = XMMatrixScaling(width, 1, length);
	const XMMATRIX translationMtx = XMMatrixTranslation(pos.x, pos.y, pos.z);

	const XMMATRIX matrix = XMMatrixMultiply(scaleMtx, translationMtx);	// (Scale * Translate)

	Transform::UpdateColliderShaderVars(matrix);
	mPlaneMesh->Render();
}

void MeshRenderer::RenderBox(const Vec3& pos, const Vec3& size, const Vec4& color)
{
	const XMMATRIX scaleMtx = XMMatrixScaling(size.x, size.y, size.z);
	const XMMATRIX translationMtx = XMMatrixTranslation(pos.x, pos.y, pos.z);

	const XMMATRIX matrix = XMMatrixMultiply(scaleMtx, translationMtx);	// (Scale * Translate)

	Transform::UpdateColliderShaderVars(matrix, color);
	mBoxMesh->Render();
}

void MeshRenderer::BuildMeshes()
{
	mBoxMesh    = std::make_unique<ModelObjectMesh>();
	mSphereMesh = std::make_unique<ModelObjectMesh>();
	mPlaneMesh  = std::make_unique<ModelObjectMesh>();

	mBoxMesh->CreateCubeMesh(1.f, 1.f, 1.f, false, true);
	mSphereMesh->CreateSphereMesh(1.f, 12, true);
	mPlaneMesh->CreatePlaneMesh(1.f, 1.f, true);
}

void MeshRenderer::ReleaseUploadBuffers()
{
	mBoxMesh->ReleaseUploadBuffers();
	mSphereMesh->ReleaseUploadBuffers();
	mPlaneMesh->ReleaseUploadBuffers();
}

void MeshRenderer::Release()
{
	mBoxMesh    = nullptr;
	mSphereMesh = nullptr;
	mPlaneMesh  = nullptr;
}