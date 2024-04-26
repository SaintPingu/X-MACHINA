#include "stdafx.h"
#include "Wait.h"

#include "Timer.h"

Wait::Wait(float waitTime, std::function<void()> callback)
	:
	mWaitTime(waitTime),
	mWaitingCallback(callback)
{
}

BT::NodeState Wait::Evaluate()
{
	mAccTime += DeltaTime();

	if (mWaitingCallback)
		mWaitingCallback();

	if (mAccTime >= mWaitTime) {
		mAccTime = 0.f;
		return BT::NodeState::Failure;
	}

	return BT::NodeState::Wait;
}
