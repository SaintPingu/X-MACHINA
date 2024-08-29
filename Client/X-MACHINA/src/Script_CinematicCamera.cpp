#include "stdafx.h"
#include "Script_CinematicCamera.h"

#include "Object.h"
#include "InputMgr.h"
#include "Timer.h"
#include "Component/Camera.h"

#define TAKE_05

void Script_CinematicCamera::InitCameraTarget()
{
	mSpeed = 1;
	mRotSpeed = 50;
}

void Script_CinematicCamera::Start()
{
	base::Start();

#ifdef TAKE_01
	Matrix take01 = {
		-0.108137786,
		-0.172923610,
		-0.978980839,
		0.00000000,
		-0.0470300913,
		0.984542787,
		-0.168711126,
		0.00000000,
		0.993022919,
		0.0277975947,
		-0.114598967,
		0.00000000,
		139.230942,
		1.48593879,
		231.386612,
		1.00000000,
	};
	mObject->SetWorldTransform(take01);
#endif

#ifdef TAKE_02
	Matrix take02 = {
		-0.395182,
		-0.0112825,
		-0.918536,
		10.2908,
		0.0667426,
		0.99693,
		-0.0409598,
		-2.22985,
		0.916176,
		-0.0774917,
		-0.393216,
		-6.18269,
		290.25,
		1.8475,
		299.081,
		1,
	};
	mObject->SetWorldTransform(take02);
#endif
	
#ifdef TAKE_03
	Matrix take03 = {
		-0.878255,
		-0.0067093,
		-0.478119,
		0,
		0.446953,
		0.343827,
		-0.825823,
		0,
		0.169932,
		-0.938993,
		-0.298976,
		0,
		241.963,
		15.32,
		206.028,
		1,
	};
	mObject->SetWorldTransform(take03);
#endif

#ifdef TAKE_04
	Matrix take04 = {
		-0.741882,
		0.00473234,
		-0.670514,
		11.9677,
		0.501959,
		0.666926,
		-0.550679,
		-2.38364,
		0.444578,
		-0.745109,
		-0.497155,
		0.438763,
		143.713,
		16.7669,
		238.939,
		1,
	};
	mObject->SetWorldTransform(take04);
#endif

#ifdef TAKE_05
	Matrix take05 = {
		0.40495,
		0.0666688,
		-0.911906,
		2.19458,
		0.0316911,
		0.995715,
		0.0868694,
		-2.76716,
		0.91379,
		-0.0640782,
		0.401102,
		-11.6887,
		34.1308,
		1.49917,
		222.845,
		1,
	};
	mObject->SetWorldTransform(take05);
#endif

	MainCamera::I->GetComponent<Script_MainCamera>()->StartShake(100.f, 0.003f);
}

void Script_CinematicCamera::Update()
{
	base::Update();

	if (KEY_TAP('L'))
	{
		mIsPlaying = !mIsPlaying;
	}

#ifdef TAKE_02
	if(mIsPlaying)
	{
		float speed = 0.3f;
		mObject->MoveStrafe(speed * DeltaTime());
	}
#endif

#ifdef TAKE_03
	static float offset{};
	if (mIsPlaying) {
		offset += 1.5f * DeltaTime();

		{
			if (mCrntShakeTime > 0.f) {
				if (Vector3::IsZero(mOrigin)) {
					mOrigin = mObject->GetPosition();
				}
				mObject->SetPosition(mOrigin + mShakeOffset);
				mObject->MoveForward(-offset);
			}
			else {
				mOrigin = Vector3::Zero;
			}
		}
	}
#endif

#ifdef TAKE_04
	if (mIsPlaying)
	{
		float speed = 0.5f;
		mObject->MoveForward(-speed * DeltaTime());
	}
#endif

	if (KEY_TAP('F')) {
		mSpeed = 10;
	}
	if (KEY_TAP('G')) {
		mSpeed = 1;
	}
	if (KEY_PRESSED(VK_LEFT)) {
		mObject->Rotate(Vector3::Up, -mRotSpeed * DeltaTime());
	}
	if (KEY_PRESSED(VK_RIGHT)) {
		mObject->Rotate(Vector3::Up, mRotSpeed * DeltaTime());
	}
	if (KEY_PRESSED(VK_UP)) {
		mObject->Rotate(Vector3::Right, -mRotSpeed * DeltaTime());
	}
	if (KEY_PRESSED(VK_DOWN)) {
		mObject->Rotate(Vector3::Right, mRotSpeed * DeltaTime());
	}

	if (KEY_PRESSED('P')) {
		mObject->Rotate(Vector3::Forward, -mRotSpeed * DeltaTime());
	}
	if (KEY_PRESSED('O')) {
		mObject->Rotate(Vector3::Forward, mRotSpeed * DeltaTime());
	}

	if (KEY_PRESSED('W')) {
		mObject->MoveForward(mSpeed * DeltaTime());
	}
	if (KEY_PRESSED('S')) {
		mObject->MoveForward(-mSpeed * DeltaTime());
	}
	if (KEY_PRESSED('A')) {
		mObject->MoveStrafe(-mSpeed * DeltaTime());
	}
	if (KEY_PRESSED('D')) {
		mObject->MoveStrafe(mSpeed * DeltaTime());
	}

	if (KEY_PRESSED(VK_SHIFT)) {
		mObject->MoveUp(-mSpeed * DeltaTime());
	}
	if (KEY_PRESSED(VK_SPACE)) {
		mObject->MoveUp(mSpeed * DeltaTime());
	}

	if (KEY_TAP('C')) {
		Matrix t = mObject->GetWorldTransform();
		std::cout << t._11 << std::endl;
		std::cout << t._12 << std::endl;
		std::cout << t._13 << std::endl;
		std::cout << t._14 << std::endl;
		std::cout << t._21 << std::endl;
		std::cout << t._22 << std::endl;
		std::cout << t._23 << std::endl;
		std::cout << t._24 << std::endl;
		std::cout << t._31 << std::endl;
		std::cout << t._32 << std::endl;
		std::cout << t._33 << std::endl;
		std::cout << t._34 << std::endl;
		std::cout << t._41 << std::endl;
		std::cout << t._42 << std::endl;
		std::cout << t._43 << std::endl;
		std::cout << t._44 << std::endl;
	}
}