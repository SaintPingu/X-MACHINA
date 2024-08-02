#pragma once

class Transform;
class AnimatorController;
class GameObject;
class SkinMesh;

// 한 모델(MasterModel)의 파일(정보)을 읽을 때의 Animation 정보
struct AnimationLoadInfo {
	bool IsManualBoneCalc{};
	std::vector<sptr<SkinMesh>> SkinMeshes{};
	std::string					AnimatorControllerFile{};
};

// Animation의 재생 및 상태 전이 등의 전반을 관리한다.
class Animator {
private:
	bool mIsCloned{};
	sptr<AnimatorController> mController{};

	GameObject* mAvatar{};
	std::unordered_map<std::string, Transform*>	mBoneFrames{};
	std::unordered_map<std::string, int>		mBoneIndices{};
	std::vector<sptr<SkinMesh>>	mSkinMeshes{};

public:
	Animator(rsptr<const AnimationLoadInfo> animationInfo, GameObject* avatar);

	AnimatorController* GetController() const { return mController.get(); }
	const Transform* GetBoneFrame(const std::string& frame) const { return mBoneFrames.at(frame); }

public:
	void UpdateShaderVariables();

	void Animate();
	void UpdateTransform();
	void CloneBoneFrames(class Object* other);

private:
	void InitController(rsptr<const AnimationLoadInfo> animationInfo);
	void InitBoneFrames(size_t skinMeshCount, GameObject* avatar, bool isManualBoneCalc);
};
