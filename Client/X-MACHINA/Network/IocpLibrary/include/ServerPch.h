#pragma once

#include "Types.h"
#include "ServerMacro.h"
#include "ThreadLocalStorage.h"
#include "Container.h"

/// +------------------------
///			COMMON
/// ------------------------+
#pragma region C++ Library
#include <windows.h>
#include <iostream>
#include <functional>
#include <cassert>
#pragma endregion

/// +------------------------
///			WINSOCK
/// ------------------------+
#pragma region WinSock 
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma endregion

#include "Lock.h"
#include "NetObjectPool.h"
#include "TypeCast.h"
#include "NetMemory.h"
#include "SendBuffer.h"
#include "Session.h"
#include "JobQueue.h"

#include "PacketSession.h"
