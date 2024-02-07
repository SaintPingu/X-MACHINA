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

	rsptr< AnimatorController> GetController() const { return mController; }

public:
	void UpdateShaderVariables();

	void Animate();

	void SetValue(const std::string& paramName, bool value);
	void SetValue(const std::string& paramName, int value);
	void SetValue(const std::string& paramName, float value);

private:
	void InitController(rsptr<const AnimationLoadInfo> animationInfo);
	void InitBoneFrames(size_t skinMeshCount, GameObject* avatar);
};
