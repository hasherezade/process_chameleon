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

bool setup_ustring(UNICODE_STRING *uStr, wchar_t *wstr)
{
    size_t wstr_len = wcslen(wstr);
    size_t wLen = wstr_len * 2;
    if (wstr_len > uStr->MaximumLength) {
        std::cerr << "The input string is too long" << std::endl;
        return false;
    }
    std::wcout << "Target: " << (wchar_t*) wstr << std::endl;
    std::wcout << "Buffer before: " << (wchar_t*) uStr->Buffer << std::endl;

    memset(uStr->Buffer, 0, uStr->MaximumLength * sizeof(wchar_t));
    
    memcpy(uStr->Buffer, wstr, wLen);
    std::wcout << "Buffer after: " << (wchar_t*) uStr->Buffer << std::endl;
    uStr->Length = wLen;
    return true;
}

PRTL_USER_PROCESS_PARAMETERS overwrite_params(PPEB local_peb, LPWSTR targetPath)
{
    PRTL_USER_PROCESS_PARAMETERS new_params = (PRTL_USER_PROCESS_PARAMETERS) VirtualAlloc(nullptr,sizeof(RTL_USER_PROCESS_PARAMETERS), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!new_params) {
        return nullptr;
    }
    PRTL_USER_PROCESS_PARAMETERS params = local_peb->ProcessParameters;
    memcpy(new_params,params, sizeof(RTL_USER_PROCESS_PARAMETERS));

    if (!setup_ustring(&new_params->ImagePathName, targetPath)) return nullptr;
    if (!setup_ustring(&new_params->CommandLine, targetPath)) return nullptr;
    return new_params;
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
    /*
    PRTL_USER_PROCESS_PARAMETERS params = create_process_params(myPeb, targetPath);
    if (params == nullptr) {
        std::cerr << "Creating params failed!" << std::endl;
        return -1;
    }

    bool is_ok = update_my_peb(myPeb, params);
    if (!is_ok) {
        return -1;
    }
    */
    PRTL_USER_PROCESS_PARAMETERS params = overwrite_params(myPeb, targetPath);
    bool is_ok = update_my_peb(myPeb, params);
    if (!is_ok) {
        return -1;
    }
    wchar_t* params_img_base = (wchar_t*) targetPath;
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
