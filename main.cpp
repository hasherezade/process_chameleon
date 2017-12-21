#include <Windows.h>

#include <iostream>
#include <stdio.h>

#include "ntddk.h"
#include "ntdll_undoc.h"
#include "util.h"
#include "peb_lookup.h"

#include <psapi.h>

#pragma comment(lib, "Ntdll.lib")

bool update_my_peb(PPEB local_peb, PRTL_USER_PROCESS_PARAMETERS new_params)
{
    if (!memcpy(&local_peb->ProcessParameters, &new_params, sizeof(PVOID))) {
        std::cout << "Cannot update Params!" << std::endl;
        return false;
    }
    return true;
}

PRTL_USER_PROCESS_PARAMETERS create_process_params(PPEB local_peb, LPWSTR targetPath)
{
    if (local_peb == nullptr || targetPath == nullptr) {
        return nullptr;
    }
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
        &(local_peb->ProcessParameters->Environment),
        (PUNICODE_STRING) &uWindowName,
        &(local_peb->ProcessParameters->DesktopInfo),
        &(local_peb->ProcessParameters->ShellInfo),
        &(local_peb->ProcessParameters->RuntimeData),
        RTL_USER_PROC_PARAMS_NORMALIZED
    );
    if (status != STATUS_SUCCESS) {
        std::cerr << "RtlCreateProcessParametersEx failed" << std::endl;
        return nullptr;
    }
    return params;
}

int wmain()
{
    if (init_ntdll_func() == false) {
        return -1;
    }
    wchar_t calcPath[MAX_PATH] = { 0 };
    ExpandEnvironmentStringsW(L"%SystemRoot%\\system32\\calc.exe", calcPath, MAX_PATH);
    wchar_t *targetPath = calcPath;

    wchar_t my_name[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, my_name, MAX_PATH);

    PTEB myTeb = NtCurrentTeb();
    PPEB myPeb = myTeb->ProcessEnvironmentBlock;

    PRTL_USER_PROCESS_PARAMETERS params = create_process_params(myPeb, targetPath);
    if (params == nullptr) {
        std::cerr << "Creating params failed!" << std::endl;
        return -1;
    }

    bool is_ok = update_my_peb(myPeb, params);
    if (!is_ok) {
        return -1;
    }

    wchar_t* params_img_base = (wchar_t*) params->ImagePathName.Buffer;
    if (!set_module_name(params_img_base)) {
        return -1;
    }

    MessageBoxW(GetDesktopWindow(), L"My momma calls me calc :D", L"Hello", MB_OK);

    //read the read path:
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
