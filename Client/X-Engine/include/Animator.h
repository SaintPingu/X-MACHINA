#pragma once

class Transform;
class AnimatorController;
class AnimationSet;
class GameObject;
class SkinMesh;

// �� ��(MasterModel)�� ����(����)�� ���� ���� Animation ����
struct AnimationLoadInfo {
	bool IsManualBoneCalc{};
	std::vector<sptr<SkinMesh>> SkinMeshes{};
	std::string					AnimatorControllerFile{};
};
     
// Animation�� ��� �� ���� ���� ���� ������ �����Ѵ�.
class Animator {
private:
	sptr<AnimatorController> mController{};

	std::vector<Transform*>		mBoneFrames{};
	std::vector<Transform*>		mFrames{};
	std::vector<sptr<SkinMesh>>	mSkinMeshes{};
	std::function<void()>       mUpdateTransformFunc{};

public:
	Animator(rsptr<const AnimationLoadInfo> animationInfo, GameObject* avatar);
	~Animator();

	rsptr<AnimatorController> GetController() const { return mController; }

public:
	void UpdateShaderVariables();

	void Animate();
	void UpdateTransform();
	void UpdateTransformManual();

private:
	void InitController(rsptr<const AnimationLoadInfo> animationInfo);
	void InitBoneFrames(size_t skinMeshCount, GameObject* avatar, bool isManualBoneCalc);
};
