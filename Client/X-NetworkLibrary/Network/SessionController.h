#pragma once

/// +-----------------------------------------------
///					SessionController 
/// ________________________________________________
///	���ӵ� ���ǵ��� �����մϴ�.
/// SessionController Ŭ������ NetworkInterface ������ 
/// ����մϴ�.
/// ���� �� Ŭ������ ����ϱ� ���ؼ��� NetworkInterface �� ���ľ��մϴ�.
/// �̷��� ���� ������ Session�� NetworkInterface ���� �����ϰ� �־��µ�
/// Session�� �����ϴ� �Լ����� �������� �Ը� Ŀ�����Դϴ�.
///					
/// ----------------------------------------------+

class NetworkInterface;
class SessionController
{
	//USE_LOCK;
	Lock::SRWLockGuard mSRWLock{};

public:
	enum class EnumInfo
	{
		Max_Session_Limit_Count = 5000,
	};

private:
	Concurrency::concurrent_unordered_map<UINT32, SPtr_Session> mSessionsMap; // Key : ID / Value : Session ( Shared Ptr )

	std::atomic_int		mCurrSessionCnt = 0;  /* ���� ���� ���� �� */
	UINT32				mMaxSessionCnt  = 0;  /* �ִ� ���� ���� �� */
	std::mutex		    mSessionsMutex  = {}; /* ���� ���� ���ؽ�	 */
	Lock::RWLock	    mSessionRWLock  = {}; /* ���� ���� ���ؽ� (RWLock) */

	std::function<SPtr_Session(void)>	mSessionConstructorFunc = {}; // Session ������ �Լ� ������ ( session���Ŭ���� ������ )

public:
	SessionController();
	~SessionController();

public:
	/* Create / Add / Release - Session */
	SPtr_Session	CreateSession(SPtr_NI netInterfaceOwner);
	void			AddSession(UINT32 sessionID, SPtr_Session session);
	void			ReleaseSession(UINT32 sessionID);

	/* Send / Broadcast - SendPacket */
	void			Broadcast(SPtr_SendPktBuf sendBuf);
	void			Send(UINT32 sessionID, SPtr_SendPktBuf sendBuf);

	/* Set Variable */
	void SetMaxSessionCnt(UINT32 sessionLimit) { mMaxSessionCnt = sessionLimit; }
	void SetSessionConstructorFunc(std::function<SPtr_Session(void)> func) { mSessionConstructorFunc = func; }

	/* Get Variable */
	UINT32 GetCurSessionCnt() { return mCurrSessionCnt; }
	UINT32 GetMaxSEssionCnt() { return mMaxSessionCnt; }

};

