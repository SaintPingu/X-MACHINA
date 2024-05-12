/// +-----------------------
///	  Memory Leack Checker
/// Must be describe first (LIFO)
/// -----------------------+
#if defined(_DEBUG)
#include <memory>
static void CheckMemoryLeak()
{
	_CrtDumpMemoryLeaks();
}
static int kMemoryChecker = std::atexit(CheckMemoryLeak);
#endif


#include "stdafx.h"
#include "X-MACHINA.h"
#include "GameFramework.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	GameFramework::I->Init(hInstance, 800, 500);
	GameFramework::I->Launch();
	GameFramework::I->Release();

	return 0;
}
