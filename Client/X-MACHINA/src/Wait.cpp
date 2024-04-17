#include "stdafx.h"
#include "Wait.h"

#include "Timer.h"

BT::NodeState Wait::Evaluate()
{
	mAccTime += DeltaTime();

	if (mAccTime >= mWaitTime) {
		mAccTime = 0.f;
		return BT::NodeState::Failure;
	}

	return BT::NodeState::Wait;
}
