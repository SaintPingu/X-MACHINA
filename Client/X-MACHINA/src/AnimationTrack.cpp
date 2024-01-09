#include "stdafx.h"
#include "AnimationTrack.h"

#include "Timer.h"

void AnimationTrack::Init()
{
	mSpeed     = 1.f;
	mPosition  = 0.0f;
	mWeight    = 1.0f;
	mClipIndex = 0;
}

void AnimationTrack::Animate(float maxLength)
{
	mPosition += mSpeed * DeltaTime();
	if (mPosition > maxLength) {
		mPosition = 0.f;
	}
}
