#pragma once
enum class HumanBone : DWORD {
	None		= 0x00,
	Root		= 0x01,
	Body		= 0x02,
	LeftArm     = 0x04,
	RightArm    = 0x08,
	LeftLeg     = 0x10,
	RightLeg    = 0x20,
	Head        = 0x40,
	LeftHand    = 0x80,
	RightHand   = 0x100,
	All			= 0xFFFFFFFF
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