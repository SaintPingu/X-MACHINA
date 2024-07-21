#pragma once

class BattleScene;
class ModelObjectMesh;

// bounds�� mesh�� �������ϱ� ���� Ŭ���� (for debug)
// texture(UV) ������ ������ �ʴ´�.
class MeshRenderer {
	friend BattleScene;		// ����� Scene Ŭ�������� �����Ѵ�.

private:
	static uptr<ModelObjectMesh> mBoxMesh;
	static uptr<ModelObjectMesh> mSphereMesh;
	static uptr<ModelObjectMesh> mPlaneMesh;

public:
	MeshRenderer() = delete;

public:
	static void Render(const BoundingBox& box, Vec4 color = Vec4::Zero);
	static void Render(const BoundingOrientedBox& box, Vec4 color = Vec4::Zero);
	static void Render(const BoundingSphere& bs, Vec4 color = Vec4::Zero);

	static void RenderPlane(const Vec3& pos, float width, float length);
	static void RenderBox(const Vec3& pos, const Vec3& size, const Vec4& color = Vec4{1.f, 0.f, 0.f, 1.f});

private:
	static void BuildMeshes();
	static void ReleaseUploadBuffers();
	static void Release();
};