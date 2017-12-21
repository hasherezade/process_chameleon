#include "util.h"
#include <iostream>

wchar_t* get_file_name(wchar_t *full_path)
{
    size_t len = wcslen(full_path);
    for (size_t i = len - 2; i >= 0; i--) {
        if (full_path[i] == '\\' || full_path[i] == '/') {
            return full_path + (i + 1);
        }
    }
    return full_path;
}

wchar_t* get_directory(IN wchar_t *full_path, OUT wchar_t *out_buf, IN const size_t out_buf_size)
{
    memset(out_buf, 0, out_buf_size);
    memcpy(out_buf, full_path, out_buf_size);

    wchar_t *name_ptr = get_file_name(out_buf);
    if (name_ptr != nullptr) {
        *name_ptr = '\0'; //cut it
    }
    return out_buf;
}