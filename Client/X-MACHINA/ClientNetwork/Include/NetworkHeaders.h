
/// +-----------------------------------
///		   네트워크 전용 헤더 파일 
/// -----------------------------------+


/* Winsock */
#pragma region WinSock - Network
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

/* Network */
#include <iphlpapi.h>				
#pragma comment(lib, "iphlpapi.lib")
#pragma endregion

/* Window */
#include <iostream>
#include <windows.h>

/* Memory */
#include <malloc.h> 
#include <memory.h>
#include <functional>

/* STL */
#include <vector>
#include <queue>
#include <unordered_map>

/* Lock */
#include <mutex>
#include <synchapi.h> // SRWLock ( slim Read Swrite Lock )  
#include <ranges>


/* Shared ptr */
using SPtr_Session           = std::shared_ptr<class Session>;
using SPtr_SendPktBuf        = std::shared_ptr<class PacketSendBuf>;
using SPtr_RecvPktBuf        = std::shared_ptr<class PacketRecvBuf>;
using SPtr_NetObj            = std::shared_ptr<class NetworkObject>;
using SPtr_Listener          = std::shared_ptr<class Listener>;
using SPtr_NI                = std::shared_ptr<class NetworkInterface>;
using SPtr_ServerNetwork     = std::shared_ptr<class ServerNetwork>;
using SPtr_ClientNetwork     = std::shared_ptr<class ClientNetwork>;
using SPtr_SendBufFactory    = std::shared_ptr<class SendBuffersFactory>;
using SPtr_PacketSendBuf     = std::shared_ptr<class PacketSendBuf>;
using SPtr_SessionController = std::shared_ptr<class SessionController>;
using SPtr_SListMemoryPool   = std::shared_ptr<class SListMemoryPool>;


/* Server Library Headers */
#include "Session.h"
#include "PacketHeader.h"

