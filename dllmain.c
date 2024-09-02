// dllmain.cpp : Defines the entry point for the DLL application.

#pragma once
#include "version.h"

#ifdef _WIN64
#define DLLPATH "\\\\.\\GLOBALROOT\\SystemRoot\\System32\\version.dll"
#else
#define DLLPATH "\\\\.\\GLOBALROOT\\SystemRoot\\SysWOW64\\version.dll"
#endif // _WIN64

#pragma comment(linker, "/EXPORT:GetFileVersionInfoA=" DLLPATH ".GetFileVersionInfoA")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoByHandle=" DLLPATH ".GetFileVersionInfoByHandle")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoExA=" DLLPATH ".GetFileVersionInfoExA")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoExW=" DLLPATH ".GetFileVersionInfoExW")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeA=" DLLPATH ".GetFileVersionInfoSizeA")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeExA=" DLLPATH ".GetFileVersionInfoSizeExA")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeExW=" DLLPATH ".GetFileVersionInfoSizeExW")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeW=" DLLPATH ".GetFileVersionInfoSizeW")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoW=" DLLPATH ".GetFileVersionInfoW")
#pragma comment(linker, "/EXPORT:VerFindFileA=" DLLPATH ".VerFindFileA")
#pragma comment(linker, "/EXPORT:VerFindFileW=" DLLPATH ".VerFindFileW")
#pragma comment(linker, "/EXPORT:VerInstallFileA=" DLLPATH ".VerInstallFileA")
#pragma comment(linker, "/EXPORT:VerInstallFileW=" DLLPATH ".VerInstallFileW")
#pragma comment(linker, "/EXPORT:VerLanguageNameA=" DLLPATH ".VerLanguageNameA")
#pragma comment(linker, "/EXPORT:VerLanguageNameW=" DLLPATH ".VerLanguageNameW")
#pragma comment(linker, "/EXPORT:VerQueryValueA=" DLLPATH ".VerQueryValueA")
#pragma comment(linker, "/EXPORT:VerQueryValueW=" DLLPATH ".VerQueryValueW")

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        init();
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Load, hinstDLL, 0, 0);
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


