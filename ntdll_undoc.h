#pragma once

#include <Windows.h>
#include "ntddk.h"
#include "ntdll_types.h"

//Structures:

typedef struct _PROCESS_BASIC_INFORMATION_WOW64
{
    NTSTATUS ExitStatus;
    ULONG64  PebBaseAddress;
    ULONG64  AffinityMask;
    KPRIORITY BasePriority;
    ULONG64  UniqueProcessId;
    ULONG64  InheritedFromUniqueProcessId;

} PROCESS_BASIC_INFORMATION_WOW64, *PPROCESS_BASIC_INFORMATION_WOW64;

typedef struct _UNICODE_STRING_WOW64 {
    USHORT Length;
    USHORT MaximumLength;
    PVOID64 Buffer;
} UNICODE_STRING_WOW64;

typedef struct _CURDIR64
{
    UNICODE_STRING_WOW64 DosPath;
    HANDLE Handle;
} CURDIR64, *PCURDIR64;

typedef struct _RTL_USER_PROCESS_PARAMETERS64
{
    ULONG MaximumLength;                            // Should be set before call RtlCreateProcessParameters
    ULONG Length;                                   // Length of valid structure
    ULONG Flags;                                    // Currently only PPF_NORMALIZED (1) is known:
                                                    //  - Means that structure is normalized by call RtlNormalizeProcessParameters
    ULONG DebugFlags;
    PVOID64 ConsoleHandle;                            // HWND to console window associated with process (if any).
    ULONG ConsoleFlags;
    DWORD64 StandardInput;
    DWORD64 StandardOutput;
    DWORD64 StandardError;
    CURDIR64 CurrentDirectory;                        // Specified in DOS-like symbolic link path, ex: "C:/WinNT/SYSTEM32"
    
    UNICODE_STRING_WOW64 DllPath;                         // DOS-like paths separated by ';' where system should search for DLL files.
    UNICODE_STRING_WOW64 ImagePathName;                   // Full path in DOS-like format to process'es file image.
    UNICODE_STRING_WOW64 CommandLine;                     // Command line
    PVOID64 Environment;                              // Pointer to environment block (see RtlCreateEnvironment)
    ULONG StartingX;
    ULONG StartingY;
    ULONG CountX;
    ULONG CountY;
    ULONG CountCharsX;
    ULONG CountCharsY;
    ULONG FillAttribute;                            // Fill attribute for console window
    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING_WOW64 WindowTitle;
    UNICODE_STRING_WOW64 DesktopInfo;                     // Name of WindowStation and Desktop objects, where process is assigned
    UNICODE_STRING_WOW64 ShellInfo;
    UNICODE_STRING_WOW64 RuntimeData;
    RTL_DRIVE_LETTER_CURDIR CurrentDirectores[0x20];
    ULONG EnvironmentSize;
} RTL_USER_PROCESS_PARAMETERS64, *PRTL_USER_PROCESS_PARAMETERS64;

// PEB 64:
typedef struct _PEB64 {
    BYTE Reserved[16];
    PVOID64 ImageBaseAddress;
    PVOID64 LdrData;
    PRTL_USER_PROCESS_PARAMETERS64 ProcessParameters;
} PEB64, *PPEB64;

//Functions:

extern NTSTATUS (NTAPI *RtlCreateProcessParametersEx)(
    _Out_ PRTL_USER_PROCESS_PARAMETERS *pProcessParameters,
    _In_ PUNICODE_STRING ImagePathName,
    _In_opt_ PUNICODE_STRING DllPath,
    _In_opt_ PUNICODE_STRING CurrentDirectory,
    _In_opt_ PUNICODE_STRING CommandLine,
    _In_opt_ PVOID Environment,
    _In_opt_ PUNICODE_STRING WindowTitle,
    _In_opt_ PUNICODE_STRING DesktopInfo,
    _In_opt_ PUNICODE_STRING ShellInfo,
    _In_opt_ PUNICODE_STRING RuntimeData,
    _In_ ULONG Flags // pass RTL_USER_PROC_PARAMS_NORMALIZED to keep parameters normalized
);

//for 32bit process on 64bit system:

extern NTSTATUS (NTAPI *NtWow64ReadVirtualMemory64)(
    IN HANDLE ProcessHandle,
    IN PVOID64 BaseAddress,
    OUT PVOID Buffer,
    IN ULONG64 Size,
    OUT PULONG64 NumberOfBytesRead
);

extern NTSTATUS (NTAPI *NtWow64QueryInformationProcess64) (
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
);

// Initialization function:

bool init_ntdll_func(BOOL isWow64);
