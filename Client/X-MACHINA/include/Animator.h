#pragma once

class Transform;
class AnimatorController;
class AnimationSet;
class GameObject;
class SkinMesh;

// �� ��(MasterModel)�� ����(����)�� ���� ���� Animation ����
struct AnimationLoadInfo {
	std::vector<sptr<SkinMesh>> SkinMeshes{};
	std::string					AnimatorControllerFile{};
};

// Animation�� ��� �� ���� ���� ���� ������ �����Ѵ�.
class Animator {
private:
	sptr<AnimatorController> mController{};

	std::vector<std::vector<Transform*>>	mBoneFramesList{};
	std::vector<sptr<SkinMesh>>				mSkinMeshes{};

public:
	Animator(rsptr<const AnimationLoadInfo> animationInfo, GameObject* avatar);
	~Animator();

	rsptr<AnimatorController> GetController() const { return mController; }

public:
	void UpdateShaderVariables();

	void LookAt(const Vec3& target);
	void Animate();

private:
	void InitController(rsptr<const AnimationLoadInfo> animationInfo);
	void InitBoneFrames(size_t skinMeshCount, GameObject* avatar);
};
