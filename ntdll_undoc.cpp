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
) = NULL;

bool init_ntdll_func()
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

    return true;
}
