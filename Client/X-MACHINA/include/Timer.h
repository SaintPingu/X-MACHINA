#pragma once

#define timer Timer::Inst()

class Timer : public Singleton<Timer> {
	friend class Singleton;

private:
	constexpr static ULONG kMaxSampleCnt = 50;		// Maximum frame time sample count

	double			mDefaultTimeScale{};	// 기본 시간 속도
	double			mTimeScale{};			// 현재 시간 속도
	float			mTimeElapsed{};			// 이전 프레임에서 현재 프레임까지 경과 시간 (프레임 전환 시간)

	/* performance count */
	__int64			mBasePerfCnt{};
	__int64			mPausedPerfCnt{};
	__int64			mStopPerfCnt{};
	__int64			mCurrPerfCnt{};
	__int64			mLastPerfCnt{};

	float			mFrameTime[kMaxSampleCnt]{};	// frame time을 최대 kMaxSampleCnt개 까지 기록한다.
	ULONG			mSampleCnt{};					// 현재 sampling된 frame cnt (최대 kMaxSampleCnt개)

	unsigned long	mCurrFrameRate{};				// 현재 frame rate
	unsigned long	mFPS{};							// Tick() 함수가 호출될 때 마다 증가한다. 1초가 경과하면 0으로 초기화된다.
	float			mFPSTimeElapsed{};				// 한 frame의 경과시간

	bool			mIsStopped{};					// 일시정지 되었는가?

private:
	Timer();
	virtual ~Timer() = default;

public:
	float GetTotalTime() const;
	// 프레임 전환 시간를 반환한다.
	float GetTimeElapsed() const { return mTimeElapsed; }
	// 현재 frame rate를 문자열로 반환한다.
	const WCHAR* GetFrameRate() const;

public:
	// calculate frame rate
	void Tick(float lockFPS = 0.f);

	// 타이머 시작(재개)
	void Start();
	// 타이머 일시 정지
	void Stop();

	void SetTimeScale(double scale);

private:
	void QueryPerformanceFrequency(__int64& freq)
	{
		::QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	}
	void QueryPerformanceCounter(__int64& cnt)
	{
		::QueryPerformanceCounter((LARGE_INTEGER*)&cnt);
	}

	// 타이머 초기화
	void Reset();
};

// 이전 프레임에서 현재 프레임까지 경과 시간을 반환한다. (프레임 전환 시간)
float DeltaTime();
