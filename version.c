#include "version.h"

typedef struct FuncPointer {
	char name[FILENAME_MAX+1];
	unsigned char oldbytes[12];
	void* func;
} FuncPointer;

typedef struct OrgFuncs {
	FuncPointer* funcs;
	wchar_t* storage;
	void* llaP;
	size_t size;
	size_t capacity;
	size_t storage_capacity;
} OrgFuncs;

OrgFuncs hookedFuncs;


static char* getStr(int off) {
	unsigned char strs[] = { 00,107, 101, 114, 110, 101, 108, 51, 50, 46, 100, 108, 108,00,76, 111, 97, 100, 76, 105, 98, 114, 97, 114, 121, 69, 120, 65, 00 };
	return (char*)(strs + (off + 1));
}

//this function uses an area of space for string conversion
//it will return the converted string pointer
//it will do this in an arena type method
//soo dont consider using these pointers for more than immediate use.
static wchar_t* aToW(CHAR* cbuf) {
	if (hookedFuncs.storage_capacity >= (MAX_PATH * (STORAGE_COUNT - 1)))
		hookedFuncs.storage_capacity = 0;

	wchar_t* wbuf = hookedFuncs.storage + hookedFuncs.storage_capacity;
	wchar_t* buf = wbuf;
	do {
		*(buf++) = (wchar_t)(*(cbuf++));
	} while (*cbuf != '\x00');
	
	//adding an extra 00 at the end for safety ;)
	*(buf++) = '\x00';
	hookedFuncs.storage_capacity += (buf-wbuf);
	
	return wbuf;
}

uintptr_t gmb(char* pname) {
	PEB_LDR_DATA* LDRData = getLdrData();
	MLIST_ENTRY* MemOrderModList = LDRData->InMemoryOrderModuleList.Flink;

	wchar_t* tPnameW = aToW(pname);

	while (MemOrderModList->Flink->imageBase) {
		if (!sicmp((char*)MemOrderModList->modName, (char*)tPnameW, 1)) {
			tPnameW[0] = 0x00;
			return (uintptr_t)MemOrderModList->imageBase;
		}

		MemOrderModList = MemOrderModList->Flink;
	}
	return llA(pname, 0, 0);

}
__declspec(dllexport) uintptr_t pebLoadLib(char* l, HANDLE _notused, DWORD dwFlags) {
	return llA(l, _notused, dwFlags);
}

static uintptr_t llA(char* l, HANDLE _notused, DWORD dwFlags) {
	uintptr_t* _llaP = (uintptr_t*)(hookedFuncs.llaP);
	if (*_llaP == (uintptr_t)0) {
		uintptr_t lla = gpaA(getStr(0), getStr(13));
		*_llaP = lla;
	}
	uintptr_t(*clla)(char*, HANDLE, DWORD) = (uintptr_t(__cdecl*)(char*, HANDLE, DWORD)) * _llaP;
	return clla(l, 0, 0);
}

uintptr_t gpaA(char* modname, char* wAPIName)
{
	uintptr_t hModule = gmb(modname);
	if (hModule == (uintptr_t)0) {
		hModule = llA(modname, 0, 0);
		if (hModule == (uintptr_t)0) {
			return (uintptr_t)0;
		}
	}

	unsigned char* lpBase = (unsigned char*)(hModule);
	IMAGE_DOS_HEADER* idhDosHeader = (IMAGE_DOS_HEADER*)(lpBase);
	if (idhDosHeader->e_magic == 0x5A4D)
	{
#ifdef _WIN64
		IMAGE_NT_HEADERS64* inhNtHeader = (IMAGE_NT_HEADERS64*)(lpBase + idhDosHeader->e_lfanew);
#else
		IMAGE_NT_HEADERS* inhNtHeader = (IMAGE_NT_HEADERS*)(lpBase + idhDosHeader->e_lfanew);
#endif
		if (inhNtHeader->Signature == 0x4550)
		{
			IMAGE_EXPORT_DIRECTORY* iedExportDirectory = (IMAGE_EXPORT_DIRECTORY*)(lpBase + inhNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
			for (register unsigned int uiIter = 0; uiIter < iedExportDirectory->NumberOfNames; ++uiIter)
			{
				char* szNames = (char*)(lpBase + ((unsigned long*)(lpBase + iedExportDirectory->AddressOfNames))[uiIter]);
				if (!sicmp(szNames, (char*)wAPIName, 0))
				{
					UINT16 usOrdinal = ((UINT16*)(lpBase + iedExportDirectory->AddressOfNameOrdinals))[uiIter];
					return (uintptr_t)(lpBase + (UINT32)((UINT32*)(lpBase + iedExportDirectory->AddressOfFunctions))[usOrdinal]);
				}
			}
		}
	}
	return 0;
}

//TODO: add a function to select a hooked function by name

static size_t hookFuncExp(uintptr_t dst, const char* name, uintptr_t funcAddr) {
	OrgFuncs* OgFs = &hookedFuncs;

	DWORD lpflOldProtect;
	VirtualProtect((void*)dst, 0x12, PAGE_EXECUTE_READWRITE, &lpflOldProtect);
#ifdef _WIN64
	if (((char*)dst)[0] == 0xff) {
		OgFs->funcs[OgFs->capacity].func = (void*)(((uintptr_t*)((char*)dst + 6 + (uintptr_t) * ((uintptr_t*)((char*)dst + 2)))));
		*((WORD*)((char*)dst + 0)) = (WORD)0xb848;
		*((uintptr_t*)((char*)dst + 2)) = funcAddr;
		*((WORD*)((char*)dst + 10)) = (WORD)0xE0FF;
	}
	else {
		memcpy(OgFs->funcs[OgFs->capacity].oldbytes, (void*)dst, 12);
		OgFs->funcs[OgFs->capacity].func = (void*)dst;
		*((WORD*)((char*)dst + 0)) = (WORD)0xb848;
		*((uintptr_t*)((char*)dst + 2)) = funcAddr;
		*((WORD*)((char*)dst + 10)) = (WORD)0xE0FF;
	}
#else
	if (*((BYTE*)((char*)dst + 0)) == 0xff) {
		OgFs->funcs[OgFs->capacity].func = (void*)*((DWORD*)*((DWORD*)((char*)dst + 2)));
		*((BYTE*)((char*)dst + 0)) = (BYTE)0xb8;
		*((uintptr_t*)((char*)dst + 1)) = funcAddr;
		*((WORD*)((char*)dst + 5)) = (WORD)0xE0FF;
	}
	else {
		memcpy(OgFs->funcs[OgFs->capacity].oldbytes, (void*)dst, 12);
		OgFs->funcs[OgFs->capacity].func = (void*)dst;
		*((BYTE*)((char*)dst + 0)) = (BYTE)0xb8;
		*((uintptr_t*)((char*)dst + 1)) = funcAddr;
		*((WORD*)((char*)dst + 5)) = (WORD)0xE0FF;
	}
#endif

	memcpy(&(OgFs->funcs[OgFs->capacity].name), name, strlen(name));
	return OgFs->capacity++;
}

// Demo hook implementation 

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

	unsigned char pbytes[12];

	memcpy(pbytes,hookedFuncs.funcs[0].func, 12);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)mbht, hWnd, 0, 0); 
	memcpy(hookedFuncs.funcs[0].func, hookedFuncs.funcs[0].oldbytes, 12);
	int(*cdW)(HWND, LPCWSTR, LPCWSTR, HICON);
	cdW = (void*)hookedFuncs.funcs[0].func;
	int ret = cdW(hWnd, szApp, szOtherStuff,hIcon);
	memcpy(hookedFuncs.funcs[0].func,pbytes, 12);
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
BOOL _CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)mbht, 0, 0, 0);
	BOOL(*cdW)(LPCSTR, LPSECURITY_ATTRIBUTES);
	cdW = (void*)hookedFuncs.funcs[1].func;
	BOOL ret = cdW(lpPathName, lpSecurityAttributes);
	return ret;
}

//end demo hook implementation

#ifdef devmode 
__declspec(dllexport) void init() {
#else
void init() {
#endif

	//init code (sets up structs in section created as "memory")
	hookedFuncs.funcs = (FuncPointer*)allocMem(26 * sizeof(FuncPointer));
	hookedFuncs.storage = (wchar_t*)allocMem(MAX_PATH * STORAGE_COUNT);
	hookedFuncs.storage_capacity = 0;
	hookedFuncs.llaP = (void*)allocMem(sizeof(void*) * 2);
	hookedFuncs.size = 26;
	hookedFuncs.capacity = 0;
	//end init

	//not recommended (should use pRun function as to not hold loader lock)
	//this is here only for demonstration and to show that in some cases 
	//when absolutely needed.. it may be done carefully. 
	/*
	if (!wcscmp(getExeName(), L"notepad.exe") || !wcscmp(getExeName(), L"calc.exe")) {
		void* rca = 0x0;

		//if the library we are attempting to get the procedure from,
		//has not already been loaded gpaA will attempt to load it...
		//resulting in some bad things happening because,
		//we still hold the loader lock at this point...

		rca = (void*)gpaA((char*)"shell32.dll", (char*)"ShellAboutW");
		if (rca == 0x00) return;
		hookFuncExp(rca, "ShellAboutW", (uintptr_t)&_ShellAboutW, &hookedFuncs);
	}*/
}

#ifdef devmode 
__declspec(dllexport) void pRun() {
#else
void pRun() {
#endif

	//this is where your code/patches/hooks should be placed.
	
	//demonstration of a simple way to check if we are attatching to what we think we are.
	//then hooking the ShellAbout calls in shell32.dll
	if (!wcscmp(getExeName(), L"mspaint.exe") || !wcscmp(getExeName(), L"notepad.exe") || !wcscmp(getExeName(), L"calc.exe")) {
		
		//temp variable to check if we got a valid procaddress
		uintptr_t rca = 0x0;

		rca = gpaA((char*)"shell32.dll", (char*)"ShellAboutW");
		if (rca == 0x00) return;
		hookFuncExp(rca, "ShellAboutW", (uintptr_t)&_ShellAboutW);
		

		//hooking this to test/demo the hook at import table locations
		rca = gpaA((char*)"kernel32.dll", (char*)"CreateDirectoryA");
		if (rca == 0x00) return;
		hookFuncExp(rca, "CreateDirectoryA", (uintptr_t)&_CreateDirectoryA);
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