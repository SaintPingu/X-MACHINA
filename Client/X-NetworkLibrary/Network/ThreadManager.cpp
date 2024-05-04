#include "pch.h"
#include "ThreadManager.h"
#include "SendBuffersFactory.h"

/// +----------------------------------
///			  Thread Manager 
/// ----------------------------------+
#pragma region Thread Manager 
DEFINE_SINGLETON(ThreadManager);

ThreadManager::ThreadManager()
{

}

ThreadManager::~ThreadManager()
{
	TLS_MGR->Destroy();
}
void ThreadManager::RunThread(std::string threadName, std::function<void(void)> func)
{
	std::lock_guard<std::mutex> lockGuard(mLock);

	mThreads.push_back(std::thread([=]() {
		
		/* Init Thread Local Storage */
		TLS_MGR->Init_TlsInfoData(threadName);
		TLS_MGR->Init_TlsSendBufFactory("SendPacketFactory" );
		
		
		func();
		
		/* Destroy Thread Local Storage */
		auto tlsDataPtr = TLS_MGR->Get_TlsInfoData();
		SAFE_DELETE(tlsDataPtr);
		auto tlsDataPtr2 = TLS_MGR->Get_TlsSendBufFactory();
		SAFE_DELETE(tlsDataPtr2);
		
		}));

}
void ThreadManager::JoinAllThreads()
{
	/* Join All threads */
	for (std::thread& thread : mThreads)
	{
		if (thread.joinable())
			thread.join(); 
	}
	mThreads.clear();
}
#pragma endregion


/// +----------------------------------
///		Thread Local Storage Manager 
/// ----------------------------------+
#pragma region Thread Local Storage Manager 
/* static value Init */
std::atomic<UINT32> TlsMgr::NewThreadID = 1;
DEFINE_SINGLETON(ThreadLocalStorageManager);

ThreadLocalStorageManager::ThreadLocalStorageManager()
{
	for (UINT8 i = 0; i < static_cast<UINT8>(TLS::TlsIndex::end); ++i) {
		mTlsIndex[i] = ::TlsAlloc();
	}
}

ThreadLocalStorageManager::~ThreadLocalStorageManager()
{
	for (UINT8 i = 0; i < static_cast<UINT8>(TLS::TlsIndex::end); ++i) {
		::TlsFree(mTlsIndex[i]);
	}
}

bool ThreadLocalStorageManager::Init()
{
	return true;

}

/* TLS::TlsInfoData */
void ThreadLocalStorageManager::Init_TlsInfoData(std::string threadName)
{
	const UINT8 TlsIndexIdx = static_cast<UINT8>(TLS::TlsIndex::TlsInfoData);

	/// TLS 데이터 생성 
	TLS::TlsInfoData* TlsData = new TLS::TlsInfoData;
	TlsData->id               = TlsMgr::NewThreadID.fetch_add(1); // atomic 
	TlsData->threadName       = threadName;

	/* lock! */
	std::lock_guard<std::mutex> lock(mMutexArr[TlsIndexIdx]);
	::TlsSetValue(mTlsIndex[TlsIndexIdx], TlsData);
}

TLS::TlsInfoData* ThreadLocalStorageManager::Get_TlsInfoData()
{
	// 현재 쓰레드의 TLS 데이터를 가져온다. 
	const UINT8 TlsIndexIdx = static_cast<UINT8>(TLS::TlsIndex::TlsInfoData);
	return static_cast<TLS::TlsInfoData*>(::TlsGetValue(mTlsIndex[TlsIndexIdx]));
}
void ThreadLocalStorageManager::Init_TlsSendBufFactory(std::string factoryID)
{
	const UINT8 TlsIndexIdx = static_cast<UINT8>(TLS::TlsIndex::TlsSendBufferFactory);
	TLS::TlsSendPacketFactory* TlsData = new TLS::TlsSendPacketFactory;

	/* Fill Data Info */
	std::lock_guard<std::mutex> lock(mMutexArr[TlsIndexIdx]);
	TlsData->strFactoryID				= factoryID;
	TlsData->SendBufFactory				= std::make_shared<SendBuffersFactory>();
	TlsData->SendBufFactory->InitPacketMemoryPools();

	/* Set Value Tls Data */
	::TlsSetValue(mTlsIndex[TlsIndexIdx], TlsData);
}
TLS::TlsSendPacketFactory* ThreadLocalStorageManager::Get_TlsSendBufFactory()
{
	// 현재 쓰레드의 TLS 데이터를 가져온다. 
	const UINT8 TlsIndexIdx = static_cast<UINT8>(TLS::TlsIndex::TlsSendBufferFactory);
	return static_cast<TLS::TlsSendPacketFactory*>(::TlsGetValue(mTlsIndex[TlsIndexIdx]));
}
#pragma endregion