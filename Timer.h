#pragma once

#define timer Timer::Inst()

class Timer {
	SINGLETON_PATTERN(Timer)

private:
	constexpr static ULONG kMaxSampleCnt = 50; // Maximum frame time sample count

	double							mTimeScale{};
	float							mTimeElapsed{};

	__int64							mBasePerfCnt{};
	__int64							mPausedPerfCnt{};
	__int64							mStopPerfCnt{};
	__int64							mCurrPerfCnt{};
	__int64							mLastPerfCnt{};

	float							mFrameTime[kMaxSampleCnt]{};
	ULONG							mSampleCnt{};

	unsigned long					mCurrFrameRate{};
	unsigned long					mFPS{};
	float							mFPSTimeElapsed{};

	bool							mIsStopped{};

public:
	Timer();
	virtual ~Timer() = default;

	float GetTotalTime() const;
	float GetTimeElapsed() const { return mTimeElapsed; }
	const WCHAR* GetFrameRate() const;

public:
	void Tick(float lockFPS = 0.f);

	void Start();
	void Stop();

	void Reset();

private:
	void QueryPerformanceFrequency(__int64& freq)
	{
		::QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	}
	void QueryPerformanceCounter(__int64& cnt)
	{
		::QueryPerformanceCounter((LARGE_INTEGER*)&cnt);
	}
};

float DeltaTime();
