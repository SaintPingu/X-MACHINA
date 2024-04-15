#include "stdafx.h"

#include "Timer.h"

bool IncreaseDelta(float& val, float speed)
{
	if (val >= 1.f) {
		val = 1.f;
		return false;
	}

	val += speed * DeltaTime();
	return true;
}

bool DecreaseDelta(float& val, float speed)
{
	if (val <= 0.f) {
		val = 0.f;
		return false;
	}

	val -= speed * DeltaTime();
	return true;
}