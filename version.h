#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "structs.h"

#define ARR_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#define FILENAME_MAX 260
#define STORAGE_COUNT 4

#define cryptbase_prox
//#define winmm_prox
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

//#define devmode

typedef void (*mfnp)(void);

extern PPEB getPeb(void);
extern PEB_LDR_DATA* getLdrData(void);
extern wchar_t* getExeName();
extern int sicmp(char*, char*, int);
extern uintptr_t allocMem(int);

uintptr_t gpaA(char* modname, char* wAPIName);
uintptr_t gmb(char* pname);

static uintptr_t llA(char* l, HANDLE _notused, DWORD dwFlags);
__declspec(dllexport) uintptr_t pebLoadLib(char* l, HANDLE _notused, DWORD dwFlags);
__declspec(dllexport) DWORD WINAPI Load(HMODULE lpParam);

#ifdef devmode 
	__declspec(dllexport) void pRun();
	__declspec(dllexport) void init();
#else 
	void init();
	void pRun();
#endif


