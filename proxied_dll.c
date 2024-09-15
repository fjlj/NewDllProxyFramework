#include "proxied_dll.h"
#define HOOKER_IMPLEMENTATION
#include "hooker.h"

// Demo hook implementation 

int SaWind = -1;
int CdAind = -1;

//a function containing some code that I want to later spawn in its own thread
void mbht(HWND hwnd) {
	MessageBoxW(hwnd, L"HEHE... Gotcha!!!", DLL_NAME " Hijack!", MB_ICONHAND);
}

//my wrapper function for ShellAboutW
 int _ShellAboutW(HWND    hWnd,
				  wchar_t* szApp,
				  wchar_t* szOtherStuff,
				  HICON   hIcon)
{
	 unhook(hookedFuncs.funcs[SaWind]);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)mbht, hWnd, 0, 0);
	

	//define and call original function
	int(*cdW)(HWND, LPCWSTR, LPCWSTR, HICON);

	//demo using a returned index as a global variable to access desired hooked function.
	cdW = (void*)hookedFuncs.funcs[SaWind].addr;
	int ret = cdW(hWnd, szApp, szOtherStuff,hIcon);

	//repatch the original function to call us back again
	rehook(hookedFuncs.funcs[SaWind]);

	//return value recieved from original function
	return ret;
}

//a wrapper for the Ascii version of the ShellAbout call that will
//convert the strings and call the ShellAboutW wrapper. 
INT  _ShellAboutA(HWND    hWnd,
				  char* szApp,
				  char* szOtherStuff,
				  HICON   hIcon) 
{
	return _ShellAboutW(hWnd, aToW(szApp), aToW(szOtherStuff), hIcon);
}

//using this to demonstrait hooking via import table
//no need to "unpatch" call and "repatch" since we did not clobber the actual function.
BOOL _CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)mbht, 0, 0, 0);
	
	//define and call original function
	BOOL(*cdW)(LPCSTR, LPSECURITY_ATTRIBUTES);
	//demo getting a hooked function by its assigned name.
	cdW = (void*)getHookedFunc("CreateDirectoryA").addr;
	BOOL ret = cdW(lpPathName, lpSecurityAttributes);

	//return original function return value
	return ret;
}

//end demo hook implementation

#ifdef devmode 
__declspec(dllexport) void pRun() {
#else
//function ran in its own thread after dll is loaded
void pRun() {
#endif

	//this is where your code/patches/hooks should be placed.
	
	//demonstration of a simple way to check if we are attatching to what we think we are.
	//then hooking the ShellAbout calls in shell32.dll
	char* exeName = (char*)getExeName();
	if (!sicmp(exeName, (char*)aToW("mspaint.exe"), 1) || 
		!sicmp(exeName, (char*)aToW("notepad.exe"), 1) ||
		!sicmp(exeName, (char*)aToW("calc.exe"), 1)) {
		
		//apply hook at proc address, label, function address to be called instead
		SaWind = hookFunction("shell32.dll", "ShellAboutW", (void*)&_ShellAboutW);
		
		//hooking this to test/demo the hook at import table locations
		hookFunction("kernel32.dll", "CreateDirectoryA", (void*)&_CreateDirectoryA);

		//demo loading another dll via peb information
		//pebLoadLib("version.dll", 0, 0);
	}
}

//This is the thread function that is started via dll load.
//You could place all of your code/patches directly into this function
//For this demonstration I have set it up as a main that is used to call
//other functions.. this is mainly just for organization sake and readability.
__declspec(dllexport) DWORD WINAPI Load(HMODULE lpParam) {
	pRun();
	return 0;
}