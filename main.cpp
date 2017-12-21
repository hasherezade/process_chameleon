#include <Windows.h>

#include <iostream>
#include <stdio.h>

#include "ntddk.h"
#include "ntdll_undoc.h"
#include "util.h"
#include "peb_lookup.h"

#include <psapi.h>

#pragma comment(lib, "Ntdll.lib")

bool update_my_peb(PVOID params_base, ULONGLONG remote_peb_addr)
{
    const PEB dummy_peb = { 0 };
    ULONGLONG offset = (ULONGLONG)&dummy_peb.ProcessParameters - (ULONGLONG)&dummy_peb;

    // Calculate offset of the parameters
    LPVOID params_offset = (LPVOID)(remote_peb_addr + offset);

    //Write parameters address into PEB:
    SIZE_T written = 0;
    if (!memcpy(params_offset, &params_base, sizeof(PVOID))) 
    {
        std::cout << "Cannot update Params!" << std::endl;
        return false;
    }
    return true;
}

bool setup_process_parameters(LPWSTR targetPath)
{
    PTEB myTeb = NtCurrentTeb();
    PPEB myPeb = myTeb->ProcessEnvironmentBlock;
    //---
    UNICODE_STRING uTargetPath = { 0 };
    RtlInitUnicodeString(&uTargetPath , targetPath);
    //---
    wchar_t dirPath[MAX_PATH] = { 0 };
    get_directory(targetPath, dirPath, MAX_PATH);
    UNICODE_STRING uCurrentDir = { 0 };
    RtlInitUnicodeString(&uCurrentDir, dirPath);
    //---
    wchar_t dllDir[] = L"C:\\Windows\\System32";
    UNICODE_STRING uDllDir = { 0 };
    RtlInitUnicodeString(&uDllDir, dllDir);
    //---
    UNICODE_STRING uWindowName = { 0 };
    wchar_t *windowName = L"Process Transformation test!";
    RtlInitUnicodeString(&uWindowName, windowName);

    PRTL_USER_PROCESS_PARAMETERS params  = nullptr;
    NTSTATUS status = RtlCreateProcessParametersEx(
        &params,
        (PUNICODE_STRING) &uTargetPath,
        (PUNICODE_STRING) &uDllDir,
        (PUNICODE_STRING) &uCurrentDir,
        (PUNICODE_STRING) &uTargetPath,
        &(myPeb->ProcessParameters->Environment),
        (PUNICODE_STRING) &uWindowName,
        &(myPeb->ProcessParameters->DesktopInfo),
        &(myPeb->ProcessParameters->ShellInfo),
        &(myPeb->ProcessParameters->RuntimeData),
        RTL_USER_PROC_PARAMS_NORMALIZED
    );
    if (status != STATUS_SUCCESS) {
        std::cerr << "RtlCreateProcessParametersEx failed" << std::endl;
        return false;
    }
    update_my_peb(params, ULONGLONG(myPeb));
    return true;
}

int wmain()
{
    if (init_ntdll_func() == false) {
        return -1;
    }
    wchar_t calcPath[MAX_PATH] = { 0 };
    ExpandEnvironmentStringsW(L"%SystemRoot%\\system32\\calc.exe", calcPath, MAX_PATH);

    wchar_t my_name[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, my_name, MAX_PATH);

    wchar_t *targetPath = calcPath;
    
    bool is_ok = setup_process_parameters(targetPath);
    set_module_name(calcPath);
    wchar_t real_path[MAX_PATH] = { 0 };
    DWORD isSet = GetProcessImageFileNameW(NtCurrentProcess(), real_path, MAX_PATH);

    if (is_ok) {
        MessageBoxW(GetDesktopWindow(), L"My momma calls me calc :D", L"Hello", MB_OK);
        MessageBoxW(GetDesktopWindow(), real_path, L"Real path:", MB_OK);
        std::cerr << "[+] Done!" << std::endl;
    } else {
        std::cerr << "[-] Failed!" << std::endl;
        return -1;
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    return wmain();
}
