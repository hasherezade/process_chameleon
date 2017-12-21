#include "peb_lookup.h"

#include <iostream>
#include <fstream>

inline PPEB get_peb()
{
#if defined(_WIN64)
    return (PPEB)__readgsqword(0x60);
#else
    return (PPEB)__readfsdword(0x30);
/*
//alternative way to fetch it:
    LPVOID PEB = NULL;
    __asm {
        mov eax, fs:[30h]
        mov PEB, eax
    };
    return (PPEB)PEB;
*/
#endif
}

inline PLDR_MODULE get_ldr_module()
{
    PPEB peb = get_peb();
    if (peb == NULL) {
        return NULL;
    }
    PPEB_LDR_DATA ldr = peb->Ldr;
    LIST_ENTRY list = ldr->InLoadOrderModuleList;
    
    PLDR_MODULE Flink = *( ( PLDR_MODULE * )( &list ) );
    return Flink;
}

LPVOID set_module_name(LPWSTR module_name)
{
    PLDR_MODULE curr_module = get_ldr_module();
    if (curr_module != NULL && curr_module->BaseAddress != NULL) {
        curr_module->BaseDllName = module_name;
        curr_module->FullDllName = module_name;
    }
    return NULL;
}
