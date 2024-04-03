// LabProject04-6.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "X-MACHINA.h"
#include "GameFramework.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	
#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	if (framework->Init(hInstance, 1280, 960)) {
		framework->Launch();
	}

#if defined(_DEBUG)
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}
