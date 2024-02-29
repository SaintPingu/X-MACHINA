#pragma once
enum class HumanBone : DWORD {
	None		= 0x00,
	Body		= 0x01,
	LeftArm     = 0x02,
	RightArm    = 0x04,
	LeftLeg     = 0x08,
	RightLeg    = 0x10,
	Head        = 0x20,
	LeftHand    = 0x40,
	RightHand   = 0x80,
	All			= 0xFFFFFFFF
};

enum class BoneType {
	None,

	// Body
	Hips,
	Spine,
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