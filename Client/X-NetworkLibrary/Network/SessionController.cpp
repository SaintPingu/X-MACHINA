#include "pch.h"
#include "SessionController.h"

SessionController::SessionController()
{
}

SessionController::~SessionController()
{
	for (int i = 0; i < mSessionsMap.size(); ++i) {
		mSessionsMap[i] = nullptr;
	}
	mSessionsMap.clear();
}

SPtr_Session SessionController::CreateSession(SPtr_NI netInterfaceOwner)
{

	SPtr_Session session = mSessionConstructorFunc();
	session->SetOwerNetworkInterface(netInterfaceOwner);
	return session;
}

void SessionController::AddSession(UINT32 sessionID, SPtr_Session session)
{

	//mSessionsMutex.lock();
	//mSessionRWLock.lockWrite();

	//WRITE_LOCK;
	mSRWLock.LockWrite();

	if (mCurrSessionCnt < mMaxSessionCnt) {
		mCurrSessionCnt.fetch_add(1);
		mSessionsMap[sessionID] = session;
	}


	mSRWLock.UnlockWrite();
	//mSessionRWLock.unlockWrite();
	//mSessionsMutex.unlock();

}

void SessionController::ReleaseSession(UINT32 sessionID)
{
	//mSessionRWLock.lockWrite();
	//WRITE_LOCK;
	mSRWLock.LockWrite();

	auto iter = mSessionsMap.find(sessionID);
	if (iter != mSessionsMap.end()) {
		mSessionsMap.unsafe_erase(iter);
		LOG_MGR->SetColor(TextColor::BrightRed);
		LOG_MGR->Cout("Session with key ", sessionID, " removed from the map.\n");
		LOG_MGR->SetColor(TextColor::Default);

	}
	else {
		LOG_MGR->SetColor(TextColor::Magenta);
		LOG_MGR->Cout("Session with key ", sessionID, " not found in the map.\n");
		LOG_MGR->SetColor(TextColor::Default);

	}

	mSRWLock.UnlockWrite();

	//mSessionRWLock.unlockWrite();
}

void SessionController::Broadcast(SPtr_SendPktBuf sendBuf)
{
	//Lock::RWLock::GetInst()->lockWrite();		
	//mSessionRWLock.lockWrite();

	//mSessionsMutex.lock();

	//WRITE_LOCK;
	
	mSRWLock.LockRead();

	for (const auto& iter : mSessionsMap) {
		SPtr_Session session = iter.second;
		iter.second->Send(sendBuf);
	}
	mSRWLock.UnlockRead();

	//mSessionRWLock.unlockWrite();
	//Lock::RWLock::GetInst()->unlockWrite();

	//mSessionsMutex.unlock();
}

void SessionController::Send(UINT32 sessionID, SPtr_SendPktBuf sendBuf)
{
	mSRWLock.LockRead();

	auto se = mSessionsMap.find(sessionID);
	if (se != mSessionsMap.end()) {
		se->second->Send(sendBuf);
	}

	mSRWLock.UnlockRead();

}
