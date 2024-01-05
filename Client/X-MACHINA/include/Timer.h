#pragma once

#define timer Timer::Inst()

class Timer : public Singleton<Timer> {
	friend class Singleton;

private:
	constexpr static ULONG kMaxSampleCnt = 50;		// Maximum frame time sample count

	double			mDefaultTimeScale{};	// �⺻ �ð� �ӵ�
	double			mTimeScale{};			// ���� �ð� �ӵ�
	float			mTimeElapsed{};			// ���� �����ӿ��� ���� �����ӱ��� ��� �ð� (������ ��ȯ �ð�)

	/* performance count */
	__int64			mBasePerfCnt{};
	__int64			mPausedPerfCnt{};
	__int64			mStopPerfCnt{};
	__int64			mCurrPerfCnt{};
	__int64			mLastPerfCnt{};

	float			mFrameTime[kMaxSampleCnt]{};	// frame time�� �ִ� kMaxSampleCnt�� ���� ����Ѵ�.
	ULONG			mSampleCnt{};					// ���� sampling�� frame cnt (�ִ� kMaxSampleCnt��)

	unsigned long	mCurrFrameRate{};				// ���� frame rate
	unsigned long	mFPS{};							// Tick() �Լ��� ȣ��� �� ���� �����Ѵ�. 1�ʰ� ����ϸ� 0���� �ʱ�ȭ�ȴ�.
	float			mFPSTimeElapsed{};				// �� frame�� ����ð�

	bool			mIsStopped{};					// �Ͻ����� �Ǿ��°�?

private:
	Timer();
	virtual ~Timer() = default;

public:
	float GetTotalTime() const;
	// ������ ��ȯ �ð��� ��ȯ�Ѵ�.
	float GetTimeElapsed() const { return mTimeElapsed; }
	// ���� frame rate�� ���ڿ��� ��ȯ�Ѵ�.
	const WCHAR* GetFrameRate() const;

public:
	// calculate frame rate
	void Tick(float lockFPS = 0.f);

	// Ÿ�̸� ����(�簳)
	void Start();
	// Ÿ�̸� �Ͻ� ����
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

	// Ÿ�̸� �ʱ�ȭ
	void Reset();
};

// ���� �����ӿ��� ���� �����ӱ��� ��� �ð��� ��ȯ�Ѵ�. (������ ��ȯ �ð�)
float DeltaTime();
