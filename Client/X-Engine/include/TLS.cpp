#include "EnginePch.h"
#include "TLS.h"

thread_local UINT32				TLS_Engine_ThreadID = 0;
thread_local UINT64				TLS_Engine_EndTickCount = 0;