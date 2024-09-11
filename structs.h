#pragma once
typedef struct MBitField {
    UCHAR ImageUsesLargePages : 1;           //0x3
    UCHAR IsProtectedProcess : 1;            //0x3
    UCHAR IsImageDynamicallyRelocated : 1;   //0x3
    UCHAR SkipPatchingUser32Forwarders : 1;  //0x3
    UCHAR IsPackagedProcess : 1;             //0x3
    UCHAR IsAppContainer : 1;                //0x3
    UCHAR IsProtectedProcessLight : 1;       //0x3
    UCHAR IsLongPathAwareProcess : 1;        //0x3
} MBitField;

typedef struct {
    int cf_flags;  /* bitmask of CO_xxx flags relevant to future */
    int cf_feature_version;  /* minor Python version (PyCF_ONLY_AST) */
} PyCompilerFlags;
#define PY_MINOR_VERSION        14

typedef struct _MLIST_ENTRY {
    struct _MLIST_ENTRY* Flink;
    struct _MLIST_ENTRY* Blink;
    void* reserved[2];
    uintptr_t imageBase;
    uintptr_t entryPoint;
    ULONG     modSize;
    DWORD     unk1[2]; //possibly mem security characteristics
    wchar_t* modNameWithPath;
    #ifdef _WIN64
        DWORD     unk2[2];
    #endif
    wchar_t* modName;


} MLIST_ENTRY, * PMLIST_ENTRY, PRMLIST_ENTRY;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING* PUNICODE_STRING;
typedef const UNICODE_STRING* PCUNICODE_STRING;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
    BYTE Reserved1[16];
    PVOID Reserved2[10];
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

typedef
VOID
(NTAPI* PPS_POST_PROCESS_INIT_ROUTINE) (
    VOID
    );

typedef struct _PEB_LDR_DATA {
    BYTE Reserved1[8];
    PVOID Reserved2[3];
    MLIST_ENTRY InMemoryOrderModuleList;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Reserved3[2];
    PPEB_LDR_DATA Ldr;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PVOID Reserved4[3];
    PVOID AtlThunkSListPtr;
    PVOID Reserved5;
    ULONG Reserved6;
    PVOID Reserved7;
    ULONG Reserved8;
    ULONG AtlThunkSListPtr32;
    PVOID Reserved9[45];
    BYTE Reserved10[96];
    PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
    BYTE Reserved11[128];
    PVOID Reserved12[1];
    ULONG SessionId;
} PEB, * PPEB;

typedef struct _SCMC {
    union wc {
        struct me {
            UINT8 cl : 1;
            UINT8 cp : 1;
            UINT8 cc : 5;
            UINT8 crp : 1;
        };
        UINT8 v;
    };
} SMSC, * PSCMC;

typedef struct _TEB {
    PVOID Reserved1[12];
    PPEB ProcessEnvironmentBlock;
    PVOID Reserved2[399];
    BYTE Reserved3[1952];
    PVOID TlsSlots[64];
    BYTE Reserved4[8];
    PVOID Reserved5[26];
    PVOID ReservedForOle;  // Windows 2000 only
    PVOID Reserved6[4];
    PVOID TlsExpansionSlots;
} TEB, * PTEB;

