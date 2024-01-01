#pragma once

class Scene;
class ModelObjectMesh;

// bounds의 mesh를 렌더링하기 위한 클래스 (for debug)
class MeshRenderer {
	friend Scene;

private:
	static uptr<ModelObjectMesh> mBoxMesh;
	static uptr<ModelObjectMesh> mSphereMesh;
	static uptr<ModelObjectMesh> mPlaneMesh;

public:
	MeshRenderer() = delete;

public:
	static void Render(const BoundingBox& box);
	static void Render(const BoundingOrientedBox& box);
	static void Render(const BoundingSphere& bs);
	static void RenderPlane(const Vec3& pos, float width, float length);

private:
	static void BuildMeshes();
	static void ReleaseUploadBuffers();
	static void Release();
};