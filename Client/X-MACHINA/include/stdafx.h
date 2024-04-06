#pragma once

/* orders of class functions */
/*---------------------------------------*/
class SampleClass {
public:
#pragma region C/Dtor
#pragma endregion

#pragma region Getter
#pragma endregion

#pragma region Setter
#pragma endregion
};
/*---------------------------------------*/

/* orders of header */
/*---------------------------------------*/
#pragma region Pragma
#pragma endregion


#pragma region Include
#pragma endregion


#pragma region Define
#pragma endregion


#pragma region Using
#pragma endregion


#pragma region ClassForwardDecl
#pragma endregion


#pragma region EnumClass
#pragma endregion


#pragma region Variable
#pragma endregion


#pragma region Struct
#pragma endregion


#pragma region Function
#pragma endregion


#pragma region NameSpace
#pragma endregion


#pragma region Class
#pragma endregion
/*---------------------------------------*/
#pragma region Pragma
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma endregion

/// +-----------------------
///		   Library 
/// -----------------------+
#ifdef _DEBUG
#pragma comment(lib, "Debug\\XMachinaServerLib.lib")
#else
#pragma comment(lib, "Release\\XMachinaServerLib.lib")
#endif

/// +-----------------------
///		  Engine Lib 
/// -----------------------+
#ifdef _DEBUG
#pragma comment(lib, "Debug\\X-Engine.lib")
#else 
#pragma comment(lib, "Release\\X-Engine.lib")
#endif

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


#pragma region Include
/* Windows */
#include <windows.h>
#include <Mmsystem.h>
#include <thread>

/* C */
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>
#include <crtdbg.h>
#include <comdef.h>
#include <wrl.h>
#include <shellapi.h>

/* C++ */
#include <iostream>
#include <functional>
#include <cassert>
#include <memory>
#include <string>
#include <fstream>
#include <random>
#include <functional>
#include <filesystem>
#include <type_traits>

/* STL Containers */
#include <span>
#include <array>
#include <vector>
#include <forward_list>
#include <deque>
#include <map>
#include <set>
#include <bitset>
#include <unordered_set>
#include <unordered_map>
#include <queue>

/* DirectX */
#include <d3d12.h>
#include <dxgidebug.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <D3d12SDKLayers.h>
#include <d3dx12.h>
#include "SimpleMath.h"

/* Custom */
#include "Common.h"


#include "Types.h"
#include "ServerMacro.h"
#include "ThreadLocalStorage.h"
#include "Container.h"

#include "Lock.h"
#include "NetObjectPool.h"
#include "TypeCast.h"
#include "NetMemory.h"
#include "SendBuffer.h"
#include "Session.h"
#include "JobQueue.h"
#include "Log/LogMgr.h"

#include "PacketSession.h"


#pragma endregion


#pragma region Define
#define WIN32_LEAN_AND_MEAN		// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.

/* ifdef */
#ifdef _DEBUG

// 메모리 누수 검사, new를 하고 delete를 하지 않은 경우 그 위치를 보여준다.
#define _CRTDBG_MAP_ALLOC

#ifdef _DEBUG
#ifdef UNICODE                                                                                      
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console") 
#else                                                                                                    
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")   
#endif                                                                                                   
#endif


#else

#define new new

#endif


struct Pos
{
	bool operator==(Pos& other)
	{
		return Z == other.Z && X == other.X;
	}

	bool operator!=(Pos& other)
	{
		return !(*this == other);
	}

	bool operator<(const Pos& other) const
	{
		if (Z != other.Z)
			return Z < other.Z;
		return X < other.X;
	}

	Pos operator+(Pos& other)
	{
		Pos ret;
		ret.Z = Z + other.Z;
		ret.X = X + other.X;
		return ret;
	}

	Pos& operator+=(Pos& other)
	{
		Z += other.Z;
		X += other.X;
		return *this;
	}

	INT32 Z{};
	INT32 X{};
};