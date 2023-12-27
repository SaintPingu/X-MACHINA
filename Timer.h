#pragma once
//-----------------------------------------------------------------------------
// File: Timer.h
//-----------------------------------------------------------------------------

const ULONG MAX_SAMPLE_COUNT = 50; // Maximum frame time sample count

class Timer
{
	SINGLETON_PATTERN(Timer)

public:
	Timer();
	virtual ~Timer();

	void Tick(float lockFPS = 0.0f);
	void Start();
	void Stop();
	void Reset();

	unsigned long GetFrameRate(LPTSTR string = NULL, int charSize = 0);
	float GetTimeElapsed() const { return mTimeElapsed; }
	float GetTotalTime();

private:
	double							mTimeScale{};
	float							mTimeElapsed{};

	__int64							mBasePerfCount{};
	__int64							mPausedPerfCount{};
	__int64							mStopPerfCount{};
	__int64							mCurrentPerfCount{};
	__int64							m_nLastPerfCount{};

	__int64							mPerfFreqPerSec{};

	float							mFrameTime[MAX_SAMPLE_COUNT]{};
	ULONG							m_nSampleCount{};

	unsigned long					mCrntFrameRate{};
	unsigned long					mFPS{};
	float							mFPSTimeElapsed{};

	bool							mIsStopped{};
};

float DeltaTime();