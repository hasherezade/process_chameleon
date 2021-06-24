#include <Windows.h>

#include <iostream>
#include <stdio.h>

#include "ntddk.h"
#include "ntdll_undoc.h"
#include "util.h"
#include "peb_lookup.h"

#include <psapi.h>

#pragma comment(lib, "Ntdll.lib")

template <typename PEB_TYPE, typename PARAMS_TYPE>
bool update_my_peb(PEB_TYPE local_peb, PARAMS_TYPE new_params)
{
    if (!memcpy(&local_peb->ProcessParameters, &new_params, sizeof(PVOID))) {
        std::cout << "Cannot update Params!" << std::endl;
        return false;
    }
    return true;
}

template <typename UNIC_STR_TYPE>
bool setup_ustring(UNIC_STR_TYPE *uStr, wchar_t *wstr)
{
    size_t wstr_len = wcslen(wstr);
    size_t byte_len = wstr_len * 2;
    if (byte_len > uStr->MaximumLength) {
        std::cerr << "The input string is too long" << std::endl;
        return false;
    }
    memset(uStr->Buffer, 0, uStr->MaximumLength);
    memcpy(uStr->Buffer, wstr, byte_len);
    uStr->Length = byte_len;
    return true;
}

template <typename PARAMS_TYPE>
bool overwrite_params(PARAMS_TYPE new_params, LPWSTR targetPath)
{
    if (!setup_ustring(&new_params->ImagePathName, targetPath)) return false;
    if (!setup_ustring(&new_params->CommandLine, targetPath)) return false;
    if (!setup_ustring(&new_params->WindowTitle, targetPath)) return false;

    wchar_t dirPath[MAX_PATH] = { 0 };
    get_directory(targetPath, dirPath, MAX_PATH);
    if (!setup_ustring(&new_params->CurrentDirectory.DosPath, dirPath)) return false;

    return true;
}

PPEB64 get_peb64(HANDLE hProcess, OUT PROCESS_BASIC_INFORMATION_WOW64 &pbi64)
{
    if (NtWow64QueryInformationProcess64 == nullptr) {
        return false;
    }
    //reset structure:
    memset(&pbi64,0, sizeof(PROCESS_BASIC_INFORMATION_WOW64));
    
    ULONG outLength = 0;
    NTSTATUS status = NtWow64QueryInformationProcess64(
        hProcess,
        ProcessBasicInformation,
        &pbi64,
        sizeof(PROCESS_BASIC_INFORMATION_WOW64),
        &outLength
    );
    if (status != STATUS_SUCCESS) {
        return nullptr;
    }
    return (PPEB64) pbi64.PebBaseAddress;
}

bool update_params_in_peb(bool isWow64, PPEB myPeb, wchar_t *targetPath)
{
    PPEB64 pebWow64 = nullptr;
    if (isWow64) {
        PROCESS_BASIC_INFORMATION_WOW64 pbi64 = { 0 };
        pebWow64 = get_peb64(GetCurrentProcess(), pbi64);
        if (pebWow64 == nullptr) {
            std::cerr << "Fetching PEB64 failed!" << std::endl;
            return false;
        }
        PRTL_USER_PROCESS_PARAMETERS64 params64 = pebWow64->ProcessParameters;
        if (!overwrite_params<PRTL_USER_PROCESS_PARAMETERS64>(params64, targetPath)) {
            return -1;
        }
        if (!update_my_peb(pebWow64, params64)) {
            return false;
        }
    }
    PRTL_USER_PROCESS_PARAMETERS params = myPeb->ProcessParameters;
    if (!overwrite_params<PRTL_USER_PROCESS_PARAMETERS>(params, targetPath)) {
        return -1;
    }
    if (!update_my_peb(myPeb, params)) {
        return false;
    }
    if (!set_module_name(params->ImagePathName)) {
        return false;
    }
    return true;
}

int wmain()
{
    BOOL isWow64 = FALSE;
    IsWow64Process(GetCurrentProcess(), &isWow64);
    std::cout << "IsWow64" << " : " << isWow64 << std::endl;

    if (init_ntdll_func(isWow64) == false) {
        printf("Cannot load functions!\n");
        return -1;
    }
    wchar_t calcPath[MAX_PATH] = { 0 };
    ExpandEnvironmentStringsW(L"%SystemRoot%\\system32\\calc.exe", calcPath, MAX_PATH);
    wchar_t *targetPath = calcPath;

    wchar_t my_name[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, my_name, MAX_PATH);

    PTEB myTeb = NtCurrentTeb();
    PPEB myPeb = myTeb->ProcessEnvironmentBlock;
    //>
    if (RtlEnterCriticalSection(myPeb->FastPebLock) != STATUS_SUCCESS) {
        return -2;
    }
    bool is_ok = update_params_in_peb(isWow64, myPeb, targetPath);
    RtlLeaveCriticalSection(myPeb->FastPebLock);
    //<
    if (!is_ok) {
        return -1;
    }
    MessageBoxW(GetDesktopWindow(), L"My momma calls me calc :D", L"Hello", MB_OK);

    //read the real path:
    wchar_t real_path[MAX_PATH] = { 0 };
    GetProcessImageFileNameW(NtCurrentProcess(), real_path, MAX_PATH);
    //display the real path:
    MessageBoxW(GetDesktopWindow(), real_path, L"Real path:", MB_OK);
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    return wmain();
}
