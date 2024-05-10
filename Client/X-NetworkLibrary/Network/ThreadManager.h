#pragma once
#include <thread>
#include <functional>

/// +-----------------------------------------------
///					Thread Manager 
/// ________________________________________________
///					������ ������
/// Thread ���� �� Join�� thread Manager ���� �����Ѵ�.
/// Thread �� �����Ѵ�. 
/// -----------------------------------------------+
/* Thread Manager */
#define THREAD_MGR ThreadManager::GetInst()
class ThreadManager
{
	DECLARE_SINGLETON(ThreadManager);

private:
	std::mutex					mLock = {};
	std::vector<std::thread>	mThreads = {};

public:
	ThreadManager();
	~ThreadManager();

public:
	void RunThread(std::string threadName, std::function<void(void)> func);
	void JoinAllThreads();
};




/// +-----------------------------------------------
///		     Thread Local Storage Manager 
/// ________________________________________________
///					Tls ������ 
/// Tls		 : Thread Local Storage struct 
/// MSDN	 : ( Tls Info ) https://learn.microsoft.com/ko-kr/cpp/parallel/thread-local-storage-tls?view=msvc-170
/// TlsMgr	 : Thread Local Storage Manager 
/// ----------------------------------------------+

/* TLS Data */
namespace TLS {
	enum class TlsIndex : UINT8 {
		TlsInfoData,
		TlsSendBufferFactory,
		end,
	};


	struct TlsInfoData {
		int			id;
		std::string threadName;
	};

	struct TlsSendPacketFactory {
		std::string			strFactoryID;
		SPtr_SendBufFactory SendBufFactory;
	};

}

/* TLS - Thread Local Storage */
#define TLS_MGR TlsMgr::GetInst()
using TlsMgr = class ThreadLocalStorageManager;
class ThreadLocalStorageManager 
{
	DECLARE_SINGLETON(TlsMgr);

private:
	DWORD mTlsIndex[static_cast<UINT8>(TLS::TlsIndex::end)] = {};

public:
	static std::atomic<UINT32> NewThreadID; // TLS�� Id �� �ο��ϱ� ���� atomic ���� 
	std::mutex mMutexArr[static_cast<UINT8>(TLS::TlsIndex::end)];

public:
	ThreadLocalStorageManager();
	~ThreadLocalStorageManager();

public:
	bool Init();

	/* TLS::TlsInfoData */
	void Init_TlsInfoData(std::string threadName);
	TLS::TlsInfoData* Get_TlsInfoData();

	/* TLS::TlsSendBufFactory */
	void Init_TlsSendBufFactory(std::string factoryID);
	TLS::TlsSendPacketFactory* Get_TlsSendBufFactory();

};


