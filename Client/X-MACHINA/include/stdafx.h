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

#pragma region Pragma Warning
#pragma warning(disable:4996)
#pragma endregion

/// +-----------------------
///		   Library 
/// -----------------------+
#define _NETWORK_LIB_VERSION_2
#ifdef _NETWORK_LIB_VERSION_1
	#ifdef _DEBUG
		#pragma comment(lib, "Debug\\XMachinaServerLib.lib")
	#else
		#pragma comment(lib, "Release\\XMachinaServerLib.lib")
#endif

#elif defined(_NETWORK_LIB_VERSION_2)
	#ifdef _DEBUG
		#pragma comment(lib, "ClientNetwork\\Lib\\Debug\\X-NetworkLibrary.lib")
	#else
		#pragma comment(lib, "ClientNetwork\\Lib\\Release\\X-NetworkLibrary.lib")
	#endif
#endif


/// +-----------------------
///		  Engine Lib 
/// -----------------------+
#ifdef _DEBUG
#pragma comment(lib, "Debug\\X-Engine.lib")
#else 
#pragma comment(lib, "Release\\X-Engine.lib")
#endif

#ifdef _DEBUG
#pragma comment(lib, "Debug\\xlntd.lib")
#else 
#pragma comment(lib, "Release\\xlnt.lib")
#endif

/// +------------------------
///			Excel
/// ------------------------+
#include <xlnt/xlnt.hpp>


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
#include <string>
#include <fstream>
#include <random>
#include <functional>
#include <filesystem>
#include <type_traits>
#include <numeric>
#include <ranges>

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
#include <stack>

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

#define _NETWORK_LIB_VERSION_2
#ifdef _NETWORK_LIB_VERSION_1
/* IOCP Library */
//#include "Types.h"
//#include "ServerMacro.h"
//#include "ThreadLocalStorage.h"
//#include "Container.h"
//
//#include "Lock.h"
//#include "NetObjectPool.h"
//#include "TypeCast.h"
//#include "NetMemory.h"
//#include "SendBuffer.h"
//#include "Session.h"
//#include "JobQueue.h"
//#include "Log/LogMgr.h"
//#include "PacketSession.h"

#include <concurrent_unordered_map.h>
#include <concurrent_queue.h>
#elif defined(_NETWORK_LIB_VERSION_2)

#include <concurrent_unordered_map.h>
#include <concurrent_queue.h>
#include <atomic>

#include "ClientNetwork/Include/LogManager.h"
#include "ClientNetwork/Include/NetworkHeaders.h"
#include "ClientNetwork/Include/UtilityMacro.h"
#include "ClientNetwork/Include/Lock.h"

using SPtr_ServerSession = std::shared_ptr<class ServerSession>;

#endif


#pragma endregion


#pragma region Define
#define WIN32_LEAN_AND_MEAN		// ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

/* ifdef */
#ifdef _DEBUG

// �޸� ���� �˻�, new�� �ϰ� delete�� ���� ���� ��� �� ��ġ�� �����ش�.
#define _CRTDBG_MAP_ALLOC

#ifdef _DEBUG

#ifdef UNICODE                                                                                      
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console") 
#else                                                                                                    
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")   
#endif           

#endif
         

#endif
#ifdef UNICODE                                                                                      
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console") 
#else                                                                                                    
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")   
#endif  
#pragma region Function
// [val]�� [speed] �ӵ��� �ִ� 1���� ������Ų��.
bool IncreaseDelta(float& val, float speed);
// [val]�� [speed] �ӵ��� �ּ� 0���� ���ҽ�Ų��.
bool DecreaseDelta(float& val, float speed);


#pragma region EnumClass
enum class BulletPSType : UINT8 {
	Explosion = 0,
	Contrail,
	Building,

	_count
};
enum { BulletPSTypeCount = static_cast<UINT8>(BulletPSType::_count) };

enum class WeaponType {
	HandedGun,
	AssaultRifle,
	ShotGun,
	MissileLauncher,
	Sniper,
	MineLauncher,
	_count
};

enum class WeaponName {
	// HandedGun
	H_Lock,

	// Shotgun
	DBMS,

	// AssaultRifle
	SkyLine,

	// MissileLaunchere
	Burnout,

	// Sniper
	PipeLine,

	// MineLauncher
	MineLauncher,

	_count
};

enum class EnemyState : UINT8 {
	Idle = 0,
	Walk,
	GetHit,
	Attack,
	Death,
	MoveToTarget,
	MoveToPath,
	Patrol,
};
#pragma endregion


#pragma region Variable
static const std::unordered_map<unsigned int, WeaponName> gkWeaponNameMap{
	{ Hash("H_Lock"), WeaponName::H_Lock },
	{ Hash("DBMS"), WeaponName::DBMS },
	{ Hash("SkyLine"), WeaponName::SkyLine },
	{ Hash("Burnout"), WeaponName::Burnout },
	{ Hash("PipeLine"), WeaponName::PipeLine },
	{ Hash("MineLauncher"), WeaponName::MineLauncher},
};

static const std::unordered_map<WeaponName, WeaponType> gkWeaponTypeMap{
	{WeaponName::H_Lock, WeaponType::HandedGun },
	{WeaponName::DBMS, WeaponType::ShotGun },
	{WeaponName::SkyLine, WeaponType::AssaultRifle },
	{WeaponName::Burnout, WeaponType::MissileLauncher },
	{WeaponName::PipeLine, WeaponType::Sniper },
	{WeaponName::MineLauncher, WeaponType::MineLauncher},
};
#pragma endregion


#pragma region Class
class PlayerMotion : public DwordOverloader<PlayerMotion> {
	DWORD_OVERLOADER(PlayerMotion)

	static const DWORD None = 0x00;
	static const DWORD Stand = 0x01;
	static const DWORD Sit = 0x02;
	static const DWORD Walk = 0x10;
	static const DWORD Run = 0x20;
	static const DWORD Sprint = 0x40;

	static PlayerMotion GetState(PlayerMotion movement) { return movement & 0x0F; }
	static PlayerMotion GetMotion(PlayerMotion movement) { return movement & 0xF0; }
};
#pragma endregion


#pragma region Namespace
namespace PheroDropInfo {
	constexpr int gkMaxPheroDrop = 30;
	constexpr int gkMaxLevel = 3;
}
#pragma endregion
