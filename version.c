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
} OrgFuncs;

OrgFuncs hookedFuncs;

static char* getStr(int off) {
	unsigned char strs[] = { 00,107, 101, 114, 110, 101, 108, 51, 50, 46, 100, 108, 108,00,76, 111, 97, 100, 76, 105, 98, 114, 97, 114, 121, 69, 120, 65, 00 };
	return (char*)(strs + (off + 1));
}

static wchar_t* aToW(CHAR* cbuf) {
	wchar_t* wbuf = hookedFuncs.storage;//(wchar_t*)((char*)runPatch(1) + 1451);//(wchar_t*)((char*)getRip() + OFFSET);
	wchar_t* buf = wbuf;
	do {
		*(buf++) = (wchar_t)(*(cbuf++));
	} while (*cbuf != '\x00');
	return wbuf;
}

__declspec(dllexport) uintptr_t gmb(char* pname) {
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
static uintptr_t llA(char* l, HANDLE _notused, DWORD dwFlags) {
	uintptr_t* _llaP = (uintptr_t*)(hookedFuncs.llaP);
	if (*_llaP == (uintptr_t)0) {
		//UINT64 kMbase = callFunc(_callFuncP, 3, _getModuleBaseP, 1, "kernel32.dll");
		uintptr_t lla = gpaA(getStr(0), getStr(13));
		*_llaP = lla;
	}
	uintptr_t(*clla)(char*, HANDLE, DWORD) = (uintptr_t(__cdecl*)(char*, HANDLE, DWORD)) * _llaP;
	return clla(l, 0, 0);
}

__declspec(dllexport) uintptr_t gpaA(char* modname, char* wAPIName)
{
	uintptr_t hModule = gmb(modname);
	if (hModule == (uintptr_t)0) {
		//UINT64 tlla = (_llaP == NULL ? &llA : _llaP);
		hModule = llA(modname, 0, 0);
		if (hModule == (uintptr_t)0) {
			return (uintptr_t)0;
		}
	}

	unsigned char* lpBase = (unsigned char*)(hModule);
	IMAGE_DOS_HEADER* idhDosHeader = (IMAGE_DOS_HEADER*)(lpBase);
	if (idhDosHeader->e_magic == 0x5A4D)
	{
		IMAGE_NT_HEADERS64* inhNtHeader = (IMAGE_NT_HEADERS64*)(lpBase + idhDosHeader->e_lfanew);
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

BOOL WINAPI _MyReadConsole(
	_In_     HANDLE  hConsoleInput,
	_Out_    LPVOID  lpBuffer,
	_In_     DWORD   nNumberOfCharsToRead,
	_Out_    LPDWORD lpNumberOfCharsRead,
	_In_opt_ LPVOID  pInputControl
) {
	BOOL(*mrc)(HANDLE, LPVOID, DWORD, LPDWORD, LPVOID) = (BOOL(*_cdecl)(HANDLE, LPVOID, DWORD, LPDWORD, LPVOID)) * ((uintptr_t*)hookedFuncs.funcs[0].func);
	mrc(hConsoleInput, lpBuffer, nNumberOfCharsToRead, lpNumberOfCharsRead, pInputControl);
	DWORD tlen = (DWORD)(lstrlenW((wchar_t*)lpBuffer) - 2);
	((wchar_t*)lpBuffer)[tlen] = 0x00;
	DWORD wrote = 0;
	WriteConsoleW(GetStdHandle(((DWORD)-11)), L"'", 1, &wrote, 0);
	WriteConsoleW(GetStdHandle(((DWORD)-11)), ((wchar_t*)lpBuffer), tlen, &wrote, 0);
	WriteConsoleW(GetStdHandle(((DWORD)-11)), L"' is a fantastic code! I say....\n", 33, &wrote, 0);
	wchar_t* correctP = (wchar_t*)L"yougottapatchit\r\n";
	tlen = (DWORD)lstrlenW(correctP);
	memmove((wchar_t*)lpBuffer, correctP, tlen * sizeof(wchar_t));
	*lpNumberOfCharsRead = tlen;
	return 1;
}



static size_t hookFuncExp(void* dst, const char* name, uintptr_t funcAddr, OrgFuncs* OgFs) {
	DWORD lpflOldProtect;
	VirtualProtect(dst, 0x12, PAGE_EXECUTE_READWRITE, &lpflOldProtect);
	memcpy(&(OgFs->funcs[OgFs->capacity].name), name, strlen(name));
	if (((char*)dst)[0] == 0xff) {
		OgFs->funcs[OgFs->capacity].func = (void*)(((uintptr_t*)((char*)dst + 6 + (DWORD) * ((DWORD*)((char*)dst + 2)))));
		*((WORD*)((char*)dst + 0)) = (WORD)0xb848;
		*((uintptr_t*)((char*)dst + 2)) = funcAddr;
		*((WORD*)((char*)dst + 10)) = (WORD)0xE0FF;
	}
	else {
		memcpy(OgFs->funcs[OgFs->capacity].oldbytes, dst, 12);
		OgFs->funcs[OgFs->capacity].func = (void*)dst;
		*((WORD*)((char*)dst + 0)) = (WORD)0xb848;
		*((uintptr_t*)((char*)dst + 2)) = funcAddr;
		*((WORD*)((char*)dst + 10)) = (WORD)0xE0FF;
	}

	return OgFs->capacity++;
}

BOOL _CreateDirectoryW(
	LPCWSTR lpPathName,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes
) {
	BOOL(*cdW)(LPCWSTR, LPSECURITY_ATTRIBUTES) = (BOOL(__cdecl*)(LPCWSTR, LPSECURITY_ATTRIBUTES)) * ((uintptr_t*)hookedFuncs.funcs[0].func);
	BOOL ret = cdW(lpPathName, lpSecurityAttributes);
	wchar_t copyTo[MAX_PATH];
	memcpy(&copyTo[0], lpPathName, lstrlenW(lpPathName) * sizeof(wchar_t));
	lstrcatW(&copyTo[0], L"\\version.dll");
	if ((ret || GetLastError() == ERROR_ALREADY_EXISTS) && wcsstr(lpPathName, L"onefile") != 0x00) {
		CopyFileW(L".\\version.dll", &copyTo[0], 0);
	}
	return ret;
}

void mbht(hwnd) {
	MessageBoxW(hwnd, L"HEHE... Gotcha!!!", L"WinMM Hijack!", MB_ICONHAND);
}

INT  _ShellAboutW(HWND    hWnd,
	LPCWSTR szApp,
	LPCWSTR szOtherStuff,
	HICON   hIcon
) {

	unsigned char pbytes[12] = { 0x00 };
	memcpy(pbytes,hookedFuncs.funcs[0].func, 12);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)mbht, hWnd, 0, 0); 
	memcpy(hookedFuncs.funcs[0].func, hookedFuncs.funcs[0].oldbytes, 12);
	INT(*cdW)(HWND, LPCWSTR, LPCWSTR, HICON) = (INT(__cdecl*)(HWND, LPCWSTR, LPCWSTR, HICON)) (uintptr_t*)hookedFuncs.funcs[0].func;
	INT  ret = cdW(hWnd, szApp, szOtherStuff,hIcon);
	memcpy(hookedFuncs.funcs[0].func,pbytes, 12);
	return ret;
}

__declspec(dllexport) void pRun() {
	void* rca = 0x0;
	rca = (void*)gpaA((char*)"kernel32.dll", (char*)"ReadConsoleW");
	if (rca == 0x00) return;
	hookFuncExp(rca, "ReadConsole", (uintptr_t)&_MyReadConsole, &hookedFuncs);
	//runPatch();
}
void init() {
	void* rca = 0x0;
	hookedFuncs.funcs = (FuncPointer*)allocMem(26 * sizeof(FuncPointer));
	hookedFuncs.storage = (wchar_t*)allocMem(MAX_PATH*2);
	hookedFuncs.llaP = allocMem(sizeof(void*) * 2);
	hookedFuncs.size = 26;
	hookedFuncs.capacity = 0;

	/*if (!wcscmp(getExeName(), L"main_obf_white.exe")) {
		rca = (void*)gpaA((char*)"kernel32.dll", (char*)"CreateDirectoryW");
		if (rca == 0x00) return;
		hookFuncExp(rca, "CreateDirectoryW", (uintptr_t)&_CreateDirectoryW, &hookedFuncs);
	}*/
	if (!wcscmp(getExeName(), L"calc1.exe")) {
		rca = (void*)gpaA((char*)"shell32.dll", (char*)"ShellAboutW");
		if (rca == 0x00) return;
		hookFuncExp(rca, "ShellAboutW", (uintptr_t)&_ShellAboutW, &hookedFuncs);
	}
}

DWORD WINAPI Load(HMODULE lpParam) {
	pRun();
	return 0;
}