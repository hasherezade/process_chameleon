#include "ntdll_undoc.h"

NTSTATUS (NTAPI *RtlCreateProcessParametersEx)(
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
) = nullptr;

NTSTATUS (NTAPI *NtWow64ReadVirtualMemory64)(
    IN HANDLE ProcessHandle,
    IN PVOID64 BaseAddress,
    OUT PVOID Buffer,
    IN ULONG64 Size,
    OUT PULONG64 NumberOfBytesRead
) = nullptr;

NTSTATUS (NTAPI *NtWow64QueryInformationProcess64) (
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
) = nullptr;

bool init_wow64_func(HMODULE lib)
{
    if (lib == nullptr) {
        return false;
    }
    FARPROC proc = GetProcAddress(lib, "NtWow64ReadVirtualMemory64");
    if (proc == nullptr) {
        return false;
    }
    NtWow64ReadVirtualMemory64 = (NTSTATUS (NTAPI *)(
        HANDLE,
        PVOID64,
        PVOID,
        ULONG64,
        PULONG64
    )) proc;

    proc = GetProcAddress(lib, "NtWow64QueryInformationProcess64");
    if (proc == nullptr) {
        return false;
    }
    NtWow64QueryInformationProcess64 = (NTSTATUS (NTAPI *)(
        HANDLE,
        PROCESSINFOCLASS,
        PVOID,
        ULONG,
        PULONG
    )) proc;

    return true;
}

bool init_ntdll_func(BOOL isWow64)
{
    HMODULE lib = LoadLibraryA("ntdll.dll");
    if (lib == nullptr) {
        return false;
    }
    FARPROC proc = GetProcAddress(lib, "RtlCreateProcessParametersEx");
    if (proc == nullptr) {
        return false;
    }
    RtlCreateProcessParametersEx = (NTSTATUS (NTAPI *)(
        PRTL_USER_PROCESS_PARAMETERS*,
        PUNICODE_STRING,
        PUNICODE_STRING,
        PUNICODE_STRING,
        PUNICODE_STRING,
        PVOID,
        PUNICODE_STRING,
        PUNICODE_STRING,
        PUNICODE_STRING,
        PUNICODE_STRING,
        ULONG 
    )) proc;

    if (isWow64) {
        if (!init_wow64_func(lib)) {
            return false;
        }
    }
    return true;
}
