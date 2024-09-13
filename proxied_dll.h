#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "structs.h"

//#define cryptbase_prox
#define winmm_prox
//#define version_prox

#ifdef cryptbase_prox
	#define DLL_NAME L"CryptBase"
#endif

#ifdef winmm_prox
	#define DLL_NAME L"WinMM"
#endif

#ifdef version_prox
	#define DLL_NAME L"Version"
#endif

//export of function called by DLLMain
__declspec(dllexport) DWORD WINAPI Load(HMODULE lpParam);

#ifdef devmode 
	__declspec(dllexport) void pRun();
#else 
	void pRun();
#endif


