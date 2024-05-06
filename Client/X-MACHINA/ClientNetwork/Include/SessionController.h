#pragma once

/// +-----------------------------------------------
///					SessionController 
/// ________________________________________________
///	접속된 세션들을 관리합니다.
/// SessionController 클래스는 NetworkInterface 에서만 
/// 사용합니다.
/// 따라서 이 클래스를 사용하기 위해서는 NetworkInterface 를 거쳐야합니다.
/// 이렇게 만든 이유는 Session을 NetworkInterface 에서 관리하고 있었는데
/// Session을 관리하는 함수들이 많아지고 규모가 커져서입니다.
///					
/// ----------------------------------------------+

class NetworkInterface;
class SessionController
{
	//USE_LOCK;
	Lock::SRWLock mSRWLock{};

public:
	enum class EnumInfo
	{
		Max_Session_Limit_Count = 5000,
	};

private:
	Concurrency::concurrent_unordered_map<UINT32, SPtr_Session> mSessionsMap; // Key : ID / Value : Session ( Shared Ptr )

	std::atomic_int		mCurrSessionCnt = 0;  /* 현재 접속 세션 수 */
	UINT32				mMaxSessionCnt  = 0;  /* 최대 접속 세션 수 */
	std::mutex		    mSessionsMutex  = {}; /* 세션 접속 뮤텍스	 */
	Lock::RWLock	    mSessionRWLock  = {}; /* 세션 접속 뮤텍스 (RWLock) */

	std::function<SPtr_Session(void)>	mSessionConstructorFunc = {}; // Session 생성자 함수 포인터 ( session상속클래스 생성자 )

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

