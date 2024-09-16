#pragma once

#define ARR_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#define FILENAME_MAX 260
#define STORAGE_COUNT 8

//#define devmode

typedef struct FuncPointer {
	char name[FILENAME_MAX + 1];
	unsigned char oldbytes[12];
	unsigned char newbytes[12];
	void* addr;
} FuncPointer;

typedef struct OrgFuncs {
	FuncPointer* funcs;
	wchar_t* storage;
	size_t storage_capacity;
	void* llaP;
	int size;
	int capacity;
} OrgFuncs;

OrgFuncs hookedFuncs;

#ifdef devmode
	__declspec(dllexport) void init()
#else
	//memory initialization of hooked functions structure.
	void init();
#endif
//this function uses an area of space for string conversion
//it will return the converted string pointer
//it will do this in an arena type method
//soo dont consider using these pointers for more than immediate use.
static wchar_t* aToW(CHAR* cbuf);
//retreive a hooked function via name
FuncPointer getHookedFunc(char* name);
//function to create a hook
//parameters are pretty self explanitory
//callbackFunc should be a function pointer...
//returns -1 if function fails
int hookFunction(char* moduleName, char* procName, void* callbackFunc);
static int hookFuncExp(OrgFuncs* OgFs, uintptr_t dst, const char* name, void* funcAddr);
//restore original bytes at hooked addr
//save patched bytes 
void unhook(FuncPointer func);
//restore hook with patch bytes at hooked addr
void rehook(FuncPointer func);
//use a portion of a custom segment
//which has read/write permissions to allocate some memory
extern uintptr_t allocMem(spaceneeded);
//some strings used in the PEB loader that wont show up as strings ;)
static char* getStr(int offset);

//function to compare char* to char*(3rd parameter 0) or w_char* to w_char* (3rd parameter 1) 
extern int sicmp(src, dest, step);

//self explanitory functions that use the PEB
//to retrieve various information.
extern PPEB getPeb(_);
extern PEB_LDR_DATA* getLdrData(_);
extern wchar_t* getExeName(_);

//GetProcAddress using module name, and procedure name.
//this will load the library if it is not loaded
//This uses the PEB to find the procedure, and will also use
//PEB lookup to call loadlibrary if needed.
uintptr_t gpaA(char* modname, char* wAPIName);

//GetModuleBase (Using PEB) This will also load
//the library (Using PEB) if not loaded.
uintptr_t gmb(char* pname);

//This will lookup loadlibrary from PEB and load the library
//this will call LoadLibraryExA thus dwFlags can be used to
//modify loading/search behavior.
static uintptr_t llA(char* l, HANDLE _notused, DWORD dwFlags);

//export of function to be able to "PEB Load" libraries if needed.
__declspec(dllexport) uintptr_t pebLoadLib(char* l, HANDLE _notused, DWORD dwFlags);

#ifdef HOOKER_IMPLEMENTATION

FuncPointer getHookedFunc(char* name) {

	for (int i = 0; i <= hookedFuncs.capacity; ++i) {
		if (!sicmp(hookedFuncs.funcs[i].name, name, 0))
			return hookedFuncs.funcs[i];
	}

	return (FuncPointer){"", { 0x0 }, 0x0};
}

void unhook(FuncPointer func) {

	//restore the original bytes (to be able to call the original function)
	memcpy(func.addr, func.oldbytes, 12);
	FlushInstructionCache(GetCurrentProcess(), (void*)func.addr, 12);

}

void rehook(FuncPointer func) {

	//restore the original bytes (to be able to call the original function)
	memcpy(func.addr, func.newbytes, 12);
	FlushInstructionCache(GetCurrentProcess(), (void*)func.addr, 12);

}

static int hookFuncExp(OrgFuncs* OgFs, uintptr_t dst, const char* name, void* funcAddr) {

	DWORD lpflOldProtect;
	VirtualProtect((void*)dst, 0x12, PAGE_EXECUTE_READWRITE, &lpflOldProtect);
	memcpy(OgFs->funcs[OgFs->capacity].oldbytes, (void*)dst, 12);
#ifdef _WIN64
	if (((char*)dst)[0] == 0xff) {
		OgFs->funcs[OgFs->capacity].addr = (void*)(((uintptr_t*)((char*)dst + 6 + (uintptr_t) * ((uintptr_t*)((char*)dst + 2)))));
		*((WORD*)((char*)dst + 0)) = (WORD)0xb848;
		*((uintptr_t*)((char*)dst + 2)) = (uintptr_t)funcAddr;
		*((WORD*)((char*)dst + 10)) = (WORD)0xE0FF;
	}
	else {
		OgFs->funcs[OgFs->capacity].addr = (void*)dst;
		*((WORD*)((char*)dst + 0)) = (WORD)0xb848;
		*((uintptr_t*)((char*)dst + 2)) = (uintptr_t)funcAddr;
		*((WORD*)((char*)dst + 10)) = (WORD)0xE0FF;
	}
#else
	if (*((BYTE*)((char*)dst + 0)) == 0xff) {
		OgFs->funcs[OgFs->capacity].addr = (void*)*((DWORD*)*((DWORD*)((char*)dst + 2)));
		*((BYTE*)((char*)dst + 0)) = (BYTE)0xb8;
		*((uintptr_t*)((char*)dst + 1)) = (uintptr_t)funcAddr;
		*((WORD*)((char*)dst + 5)) = (WORD)0xE0FF;
	}
	else {
		OgFs->funcs[OgFs->capacity].addr = (void*)dst;
		*((BYTE*)((char*)dst + 0)) = (BYTE)0xb8;
		*((uintptr_t*)((char*)dst + 1)) = (uintptr_t)funcAddr;
		*((WORD*)((char*)dst + 5)) = (WORD)0xE0FF;
	}
#endif
	memcpy(OgFs->funcs[OgFs->capacity].newbytes, (void*)dst, 12);
	memcpy(&(OgFs->funcs[OgFs->capacity].name), name, strlen(name));
	return OgFs->capacity++;
}

int hookFunction(char* moduleName, char* procName, void* callbackFunc) {
	if (hookedFuncs.capacity >= hookedFuncs.size) return -1;
	uintptr_t rca = 0x0;
	rca = gpaA(moduleName, procName);
	if (rca == 0x00) return -1;
	//apply hook at proc address, label, function address to be called instead
	return hookFuncExp(&hookedFuncs, rca, procName, callbackFunc);
}

#ifdef devmode 
__declspec(dllexport) void init() {
#else
void init() {
#endif
	
	//init code (sets up structs in portion of bss section created as "memory")
	hookedFuncs.funcs = (FuncPointer*)allocMem(26 * sizeof(FuncPointer));
	hookedFuncs.storage = (wchar_t*)allocMem(MAX_PATH * STORAGE_COUNT);
	hookedFuncs.storage_capacity = 0;
	hookedFuncs.llaP = (void*)allocMem(sizeof(void*) * 2);
	hookedFuncs.size = 26;
	hookedFuncs.capacity = 0;
	//end init

}

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
	hookedFuncs.storage_capacity += (buf - wbuf);

	return wbuf;
}



//PEB LOADER STUFF

static char* getStr(int off) {
	unsigned char strs[] = { 00,107, 101, 114, 110, 101, 108, 51, 50, 46, 100, 108, 108,00,76, 111, 97, 100, 76, 105, 98, 114, 97, 114, 121, 69, 120, 65, 00 };
	return (char*)(strs + (off + 1));
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
			return 0;
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


#endif