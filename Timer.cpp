//-----------------------------------------------------------------------------
// File: Timer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Timer.h"

SINGLETON_PATTERN_DEFINITION(Timer)

Timer::Timer()
{
	::QueryPerformanceFrequency((LARGE_INTEGER *)&mPerfFreqPerSec);
	::QueryPerformanceCounter((LARGE_INTEGER *)&m_nLastPerfCount); 
	mTimeScale = 1.0 / (double)mPerfFreqPerSec;

	mBasePerfCount = m_nLastPerfCount;
}

Timer::~Timer()
{
}

void Timer::Tick(float lockFPS)
{
	if (mIsStopped) {
		mTimeElapsed = 0.0f;
		return;
	}

	::QueryPerformanceCounter((LARGE_INTEGER *)&mCurrentPerfCount);
	float timeElapsed = float((mCurrentPerfCount - m_nLastPerfCount) * mTimeScale);

    if (lockFPS > 0.0f) {
        while (timeElapsed < (1.0f / lockFPS)) {
	        ::QueryPerformanceCounter((LARGE_INTEGER *)&mCurrentPerfCount);
	        timeElapsed = float((mCurrentPerfCount - m_nLastPerfCount) * mTimeScale);
        }
    } 

	m_nLastPerfCount = mCurrentPerfCount;

    if (fabsf(timeElapsed - mTimeElapsed) < 1.0f) {
        ::memmove(&mFrameTime[1], mFrameTime, (MAX_SAMPLE_COUNT - 1) * sizeof(float));
        mFrameTime[0] = timeElapsed;
        if (m_nSampleCount < MAX_SAMPLE_COUNT) m_nSampleCount++;
    }

	mFPS++;
	mFPSTimeElapsed += timeElapsed;
	if (mFPSTimeElapsed > 1.0f)  {
		mCrntFrameRate	= mFPS;
		mFPS = 0;
		mFPSTimeElapsed = 0.0f;
	} 

    mTimeElapsed = 0.0f;
	for (ULONG i = 0; i < m_nSampleCount; i++) {
		mTimeElapsed += mFrameTime[i];
	}
	if (m_nSampleCount > 0) {
		mTimeElapsed /= m_nSampleCount;
	}
}

const WCHAR* Timer::GetFrameRate()
{
	constexpr int radix = 10;
	constexpr int kBuffSize = 5;

	static WCHAR buff[kBuffSize]{L'\0'};

    _itow_s(mCrntFrameRate, buff, kBuffSize, radix);

    return buff;
}

float Timer::GetTotalTime()
{
	if (mIsStopped) return float(((mStopPerfCount - mPausedPerfCount) - mBasePerfCount) * mTimeScale);
	return float(((mCurrentPerfCount - mPausedPerfCount) - mBasePerfCount) * mTimeScale);
}

void Timer::Reset()
{
	__int64 nPerfCount;
	::QueryPerformanceCounter((LARGE_INTEGER*)&nPerfCount);

	mBasePerfCount = nPerfCount;
	m_nLastPerfCount = nPerfCount;
	mStopPerfCount = 0;
	mIsStopped = false;
}

void Timer::Start()
{
	__int64 nPerfCount;
	::QueryPerformanceCounter((LARGE_INTEGER *)&nPerfCount);
	if (mIsStopped)
	{
		mPausedPerfCount += (nPerfCount - mStopPerfCount);
		m_nLastPerfCount = nPerfCount;
		mStopPerfCount = 0;
		mIsStopped = false;
	}
}

void Timer::Stop()
{
	if (!mIsStopped)
	{
		::QueryPerformanceCounter((LARGE_INTEGER *)&mStopPerfCount);
		mIsStopped = true;
	}
}


float DeltaTime()
{
	return timer->GetTimeElapsed();
}