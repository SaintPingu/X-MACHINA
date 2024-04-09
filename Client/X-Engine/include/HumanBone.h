#pragma once
class HumanBone : public DwordOverloader<HumanBone> {
	DWORD_OVERLOADER(HumanBone)

	static const DWORD None      = 0x00;
	static const DWORD Root      = 0x01;
	static const DWORD Body      = 0x02;
	static const DWORD LeftArm   = 0x04;
	static const DWORD RightArm  = 0x08;
	static const DWORD LeftLeg   = 0x10;
	static const DWORD RightLeg  = 0x20;
	static const DWORD Head      = 0x40;
	static const DWORD LeftHand  = 0x80;
	static const DWORD RightHand = 0x100;
	static const DWORD All       = 0xFFFFFFFF;
};

enum class BoneType {
	None,

	// Root
	Hips,
	Spine,

	// Body
	Chest,
	UpperChest,

	// Left Arm
	LeftShoulder,
	LeftUpperArm,
	LeftLowerArm,
	LeftHand,

	// Right Arm
	RightShoulder,
	RightUpperArm,
	RightLowerArm,
	RightHand,

	// Left Leg
	LeftUpperLeg,
	LeftLowerLeg,
	LeftFoot,
	LeftToes,

	// Right Leg
	RightUpperLeg,
	RightLowerLeg,
	RightFoot,
	RightToes,

	// Head
	Neck,
	Head,
	LeftEye,
	RightEye,
	Jaw,

	// Left Hand
	LeftThumbProximal,
	LeftThumbIntermediate,
	LeftThumbDistal,

	LeftIndexProximal,
	LeftIndexIntermediate,
	LeftIndexDistal,

	LeftMiddleProximal,
	LeftMiddleIntermediate,
	LeftMiddleDistal,

	LeftRingProximal,
	LeftRingIntermediate,
	LeftRingDistal,

	LeftLittleProximal,
	LeftLittleIntermediate,
	LeftLittleDistal,

	// Right Hand
	RightThumbProximal,
	RightThumbIntermediate,
	RightThumbDistal,

	RightIndexProximal,
	RightIndexIntermediate,
	RightIndexDistal,

	RightMiddleProximal,
	RightMiddleIntermediate,
	RightMiddleDistal,

	RightRingProximal,
	RightRingIntermediate,
	RightRingDistal,

	RightLittleProximal,
	RightLittleIntermediate,
	RightLittleDistal,
};