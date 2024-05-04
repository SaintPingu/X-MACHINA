#pragma once


#include "NetworkHeaders.h"
/// +------------------------
///			COMMON
/// ------------------------+
#pragma region C++ Library
#include <windows.h>
#include <iostream>
#include <functional>
#include <cassert>
#pragma endregion


/* Default */
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

/* C++ */

#include <cassert>
#include <string>

/* STL */
#include <vector>
#include <array>

/* Concurrent */
#include <concurrent_queue.h>
#include <concurrent_unordered_map.h>

/* My Utility Hedaers */
#include "UtilityMacro.h"
