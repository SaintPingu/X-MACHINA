#include "EnginePch.h"
#include "Timer.h"


float DeltaTime()
{
	return Timer::I->GetTimeElapsed();
}


Timer::Timer()
{
	__int64	perfFreq{};

	QueryPerformanceFrequency(perfFreq);
	mDefaultTimeScale = 1.0 / static_cast<double>(perfFreq);
	mTimeScale        = mDefaultTimeScale;

	Reset();
}


float Timer::GetTotalTime() const
{
	if (mIsStopped) 
	{
		return static_cast<float>(((mStopPerfCnt - mPausedPerfCnt) - mBasePerfCnt) * mTimeScale);
	}

	return static_cast<float>(((mCurrPerfCnt - mPausedPerfCnt) - mBasePerfCnt) * mTimeScale);
}


const WCHAR* Timer::GetFrameRate() const
{
	constexpr int kRadix    = 10;
	constexpr int kBuffSize = 5;

	static WCHAR buff[kBuffSize]{ L'\0' };

	_itow_s(mCurrFrameRate, buff, kBuffSize, kRadix);

	return buff;
}


void Timer::Tick(float lockFPS)
{
	if (mIsStopped) {
		mTimeElapsed = 0.f;
		return;
	}

	QueryPerformanceCounter(mCurrPerfCnt);
	float timeElapsed = static_cast<float>((mCurrPerfCnt - mLastPerfCnt) * mTimeScale);

    if (lockFPS > 0.f) {
        while (timeElapsed < (1.f / lockFPS)) {
	        QueryPerformanceCounter(mCurrPerfCnt);
	        timeElapsed = static_cast<float>((mCurrPerfCnt - mLastPerfCnt) * mTimeScale);
        }
    } 

	mLastPerfCnt = mCurrPerfCnt;

    if (fabsf(timeElapsed - mTimeElapsed) < 1.f) {
        ::memmove(&mFrameTime[1], mFrameTime, (kMaxSampleCnt - 1) * sizeof(float));
        mFrameTime[0] = timeElapsed;
		if (mSampleCnt < kMaxSampleCnt) {
			mSampleCnt++;
		}
    }

	mFPS++;
	mFPSTimeElapsed += timeElapsed;
	if (mFPSTimeElapsed > 1.f)  {
		mCurrFrameRate	= mFPS;
		mFPS            = 0;
		mFPSTimeElapsed = 0.f;
	} 

    mTimeElapsed = 0.f;
	for (ULONG i = 0; i < mSampleCnt; i++) {
		mTimeElapsed += mFrameTime[i];
	}
	if (mSampleCnt > 0) {
		mTimeElapsed /= mSampleCnt;
	}
}

void Timer::Start()
{
	__int64 perfCnt;
	QueryPerformanceCounter(perfCnt);
	if (mIsStopped) {
		mPausedPerfCnt += (perfCnt - mStopPerfCnt);
		mLastPerfCnt    = perfCnt;
		mStopPerfCnt    = 0;
		mIsStopped      = false;
	}
}

void Timer::Stop()
{
	if (!mIsStopped)
	{
		QueryPerformanceCounter(mStopPerfCnt);
		mIsStopped = true;
	}
}

void Timer::SetTimeScale(double scale)
{
	mTimeScale = mDefaultTimeScale * scale;
}


void Timer::Reset()
{
	__int64 perfCnt;
	QueryPerformanceCounter(perfCnt);

	mBasePerfCnt = perfCnt;
	mLastPerfCnt = perfCnt;
	mStopPerfCnt = 0;
	mIsStopped = false;
}