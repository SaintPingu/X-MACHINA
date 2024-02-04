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

public:
	void UpdateShaderVariables();

	void Animate();

	void SetBool(const std::string& name, bool value);

private:
	void InitController(rsptr<const AnimationLoadInfo> animationInfo);
	void InitBoneFrames(size_t skinMeshCount, GameObject* avatar);
};
