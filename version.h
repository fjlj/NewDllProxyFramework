#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "structs.h"


#define ARR_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#define FILENAME_MAX 260

typedef void (*mfnp)(void);

//extern HMODULE version_dll;
extern PPEB getPeb(void);
extern void* getRip(void);
extern PEB_LDR_DATA* getLdrData(void);
extern int sicmp(char*, char*, int);
extern wchar_t* getExeName();
extern uintptr_t allocMem(int);
__declspec(dllexport) void pRun();
__declspec(dllexport) uintptr_t gpaA(char* modname, char* wAPIName);
__declspec(dllexport) uintptr_t gmb(char* pname);
//static uintptr_t llA(char* l);
static uintptr_t llA(char* l, HANDLE _notused, DWORD dwFlags);
DWORD WINAPI Load(HMODULE lpParam);
void init();